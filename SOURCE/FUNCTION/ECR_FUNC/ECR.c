#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <emv_cl.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"
#include "../../EVENT/Menu.h"
#include "../../PRINT/Print.h"
#include "../../DISPLAY/Display.h"
#include "../../EVENT/MenuMsg.h"
#include "../../EVENT/Flow.h"

#include "../Function.h"
#include "../FuncTable.h"
#include "../Sqlite.h"
#include "../Batch.h"
#include "../HDT.h"
#include "../CFGT.h"
#include "../EDC.h"
#include "../SCDT.h"
#include "../HDPT.h"
#include "../CDT.h"

#include "ECR.h"
#include "RS232.h"
#include "../USB.h"
#include "../BaseUSB.h"
#include "../KMS.h"
#include "../Accum.h"
#include "../../COMM/Ethernet.h"
#include "../../COMM/WiFi.h"

#include "../../../CTLS/CTLS.h"

#include "../../FUNCTION/ECR_FUNC/EcrPackFunc.h"
#include "../../FUNCTION/CARD_FUNC/CardFunction.h"

#include "../FILE_FUNC/File.h"
#include "../FILE_FUNC/FIleLogFunc.h"

#include "../../FUNCTION/UNIT_FUNC/TimeUint.h"

#include "../COSTCO_FUNC/Costco.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

extern int ginDebug;
extern int ginISODebug;
extern int ginDisplayDebug;
extern int ginECR_ResponseFd;
extern int ginMachineType;
extern unsigned char guszCTLSInitiOK;
ECR_TABLE gsrECROb = {};
ECR_GASSTATION_GLOBAL_TABLE gsrECR_GASStation_tb = {};

/* 抓出目前結構儲存 TransType 的值 2020/1/30 下午 4:41 [SAM] */
int inECR_ECROB_GetTransType(char* szTempTransType)
{
	memcpy(szTempTransType, gsrECROb.srTransData.szTransType, 2);
	return VS_SUCCESS;
}

/* 抓出目前結構儲存 inErrorType 的值 2020/1/30 下午 4:41 [SAM] */
int inECR_ECROB_GetErrorCode()
{
	return gsrECROb.srTransData.inErrorType;
}

/* 設定指定的Error Type 2020/2/4 下午 6:45 */
int inECR_ECROB_SetErrorCode(int inErrorType)
{
	gsrECROb.srTransData.inErrorType = inErrorType;
	return VS_SUCCESS;
}

/*
Function        :inECR_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :開機initial
 */
int inECR_Initial(void)
{
	int inRetVal = VS_ERROR;
	char szTMSOK[2 + 1];
	char szECREnable[2 + 1];
	char szECRVersion[2 + 1];
	char szComPort[4 + 1];

	/* 檢查是否做過【參數下載】 */
	memset(szTMSOK, 0x00, sizeof (szTMSOK));
	inGetTMSOK(szTMSOK);
	if (szTMSOK[0] != 'Y')
	{
		return (VS_SUCCESS);
	}

	/* 檢查是否有支援收銀機連線 */
	memset(szECREnable, 0x00, sizeof (szECREnable));
	inGetECREnable(szECREnable);
	if (szECREnable[0] != 'Y')
	{
		inDISP_DispLogAndWriteFlie(" Ecr Init Not Supore Ecr Func[%s] Line[%d]", szECREnable, __LINE__);
		return (VS_SUCCESS);
	}

	memset(&gsrECROb, 0x00, sizeof (ECR_TABLE));
	/* 設定 ECR 版本 */
	memset(szECRVersion, 0x00, sizeof (szECRVersion));
	inGetECRVersion(szECRVersion);
	gsrECROb.srSetting.inVersion = atoi(szECRVersion);

	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);

	inDISP_DispLogAndWriteFlie(" Ecr Init ECRVerson[%d]  Com[%s] Line[%d]", gsrECROb.srSetting.inVersion, szComPort, __LINE__);
	
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
		memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
		memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
		memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_ECR_Initial();
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		inRetVal = inUSB_ECR_Initial();
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		inRetVal = inBaseUSB_ECR_Initial();
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = inWiFi_ECR_Initial();
	} else
	{
		inRetVal = VS_ERROR;
	}

	/* 根據ECRVersion來決定，COMPORT的設定 */
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_ClearAll();
		inDISP_Msg_BMP(_ERR_INIT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "RS232", _LINE_8_5_);
	} else
	{
		/* 代表設定完成 */
		gsrECROb.srSetting.uszSettingOK = VS_TRUE;

		/* 重新初始化完還是要清除一下BUFFER 20190903 [SAM] */
		/* 清空接收的buffer */
		inRS232_FlushRxBuffer(gsrECROb.srSetting.uszComPort);
		inRS232_FlushTxBuffer(gsrECROb.srSetting.uszComPort);
	}

	return (VS_SUCCESS);
}

/*
Function        :inECR_FlushRxBuffer
Date&Time       :2017/6/1 下午 5:54
Describe        :清Buffer用
 */
int inECR_FlushRxBuffer()
{
	int inRetVal = VS_ERROR;
	char szComPort[4 + 1];
        
        /* 沒設定完成，不用檢查 20230330 Miyano 改在這裡檢查 */
	if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
	{
		return (VS_ERROR);
	}
        
	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
			memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
			memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
			memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_FlushRxBuffer(gsrECROb.srSetting.uszComPort);
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		inRetVal = inUSB_FlushRxBuffer();
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		inRetVal = inBaseUSB_FlushRxBuffer();
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = VS_SUCCESS;
	} else
	{
		inDISP_DispLogAndWriteFlie("  ECR Flash Comport *Error* szComPort [%s] Line[%d]", szComPort, __LINE__);
		inRetVal = VS_SUCCESS;
	}

	return (inRetVal);
}

/*
Function        :inECR_Send_Check
Date&Time       :2017/11/15 下午 5:35
Describe        :
 */
int inECR_Send_Check()
{
	int inRetVal = VS_ERROR;
	char szComPort[4 + 1];

	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
			memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
			memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
			memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		/* 沒設定完成，不用檢查 */
		if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
		{
			inRetVal = VS_ERROR;
		} else
		{
			inRetVal = inRS232_Data_Send_Check(gsrECROb.srSetting.uszComPort);
		}
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		/* 沒設定完成，不用檢查 */
		if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
		{
			inRetVal = VS_ERROR;
		} else
		{
			inRetVal = inUSB_Data_Send_Check();
		}
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		/* 沒設定完成，不用檢查 */
		if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
		{
			inRetVal = VS_ERROR;
		} else
		{
			inRetVal = inBaseUSB_Data_Send_Check();
		}
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = VS_SUCCESS;
	} else
	{
		inRetVal = VS_ERROR;
	}

	return (inRetVal);
}

/*
Function        :inECR_Receive_Check
Date&Time       :2017/6/1 下午 6:00
Describe        :用來確認ECR是否發動交易
 */
int inECR_Receive_Check(unsigned short *usLen)
{
	int inRetVal = VS_ERROR;
	char szComPort[4 + 1];

	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
		memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
		memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
		memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		/* 沒設定完成，不用檢查 */
		if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
		{
			inRetVal = VS_ERROR;
		} else
		{
                        inRetVal = inRS232_Data_Receive_Check(gsrECROb.srSetting.uszComPort, usLen);
		}
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		/* 沒設定完成，不用檢查 */
		if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
		{
			inRetVal = VS_ERROR;
		} else
		{
			inRetVal = inUSB_Data_Receive_Check(usLen);
		}
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		/* 沒設定完成，不用檢查 */
		if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
		{
			inRetVal = VS_ERROR;
		} else
		{
			inRetVal = inBaseUSB_Data_Receive_Check(usLen);
		}
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = inWiFi_IsAccept();
	} else
	{
		inRetVal = VS_ERROR;
	}

	return (inRetVal);
}

/*
Function        :inECR_Receive_Transaction
Date&Time       :2017/6/1 下午 6:13
Describe        :從ECR接收金額及交易別資料
 */
int inECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	char szComPort[4 + 1];

	/* 不是ECR連線就跳走 */
	if (pobTran->uszECRBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
		memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
		memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
		memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_ECR_Receive_Transaction(pobTran);
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		inRetVal = inUSB_ECR_Receive_Transaction(pobTran);
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		inRetVal = inBaseUSB_ECR_Receive_Transaction(pobTran);
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = inWiFi_ECR_Receive_Transaction(pobTran);
	} else
	{
		inRetVal = VS_SUCCESS;
	}

	return (inRetVal);
}

/*
Function        :inECR_Receive_Second_Transaction
Date&Time       :2018/5/28 下午 4:22
Describe        :For兩段式交易使用，第二次收
 */
int inECR_Receive_Second_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	char szComPort[4 + 1];

	/* 不是ECR連線就跳走 */
	if (pobTran->uszECRBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}

	/* 不是兩段式交易，不用收第二次 */
	if (pobTran->uszCardInquiryFirstBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	/* [國泰車麻吉]第二段交易會清空螢幕,所以多加顯示訊息 2022/2/18 [SAM] */
	inDISP_PutGraphic(_ECR_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
		memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
		memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
		memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_ECR_Receive_Transaction(pobTran);
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		inRetVal = inUSB_ECR_Receive_Transaction(pobTran);
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		inRetVal = inBaseUSB_ECR_Receive_Transaction(pobTran);
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = inWiFi_ECR_Receive_Transaction(pobTran);
	} else
	{
		inRetVal = VS_SUCCESS;
	}

	return (inRetVal);
}

/*
Function        :inECR_Load_TMK
Date&Time       :2017/11/16 下午 1:29
Describe        :
 */
int inECR_Load_TMK(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	char szComPort[4 + 1];

	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
		memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
		memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
		memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_ECR_Load_TMK_From_520(pobTran);
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		inRetVal = VS_ERROR;
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		inRetVal = VS_ERROR;
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = VS_ERROR;
	} else
	{
		inRetVal = VS_SUCCESS;
	}

	return (inRetVal);
}

/*
Function        :inECR_Send_Transaction_Result
Date&Time       :2017/6/1 下午 6:13
Describe        :印帳單前要送給ECR
 */
int inECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	char szComPort[4 + 1];
	char szFESMode[2 + 1];

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* 用區域的時間計算 */
	unsigned long ulRunTime;
	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inECR_Send_Transaction_Result INIT");

	/* 不是ECR連線就跳走 */
	if (pobTran->uszECRBit != VS_TRUE)
	{
		inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inECR_Send_Transaction_Result ECR !=TRUE END");
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] ECR != TRUE END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

#ifdef _FUBON_MAIN_HOST_
	/* 富邦自助交易會在結帳完後才回送資料，其餘交易不判斷 
	 * 但要注意如果有傳送ECR 資料回POS  srECROb->srTransData.uszIsResponce  要記得設為 VS_TRUE
	 * 不然不為自助時，會傳送二次  2020/2/4 下午 6:20 [SAM] */
	if (pobTran->uszKioskFlag == 'Y' &&
		(pobTran->srBRec.inCode == _SETTLE_ && pobTran->uszKioskSendFlag != 'Y'))
	{
		/* 設定為Kiosk 結帳傳送模式 2020/2/4 下午 6:34 */
		pobTran->uszKioskSendFlag = 'Y';
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] uszKioskFlag == Y END -----", __FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}

#endif 

	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
		memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
		memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
		memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_ECR_Send_Transaction_Result(pobTran);
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		inRetVal = inUSB_ECR_Send_Transaction_Result(pobTran);
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		inRetVal = inBaseUSB_ECR_Send_Transaction_Result(pobTran);
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = inWiFi_ECR_Send_Result(pobTran);
	} else
	{
		inRetVal = VS_SUCCESS;
	}

	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inECR_Send_Transaction_Result END");

	inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	/* 即使回傳失敗也要印簽單 */
	return (VS_SUCCESS);
}

/*
Function        :inECR_Send_Inquiry_Result
Date&Time       :2018/5/16 下午 5:16
Describe        :回傳查詢卡號結果
 */
int inECR_Send_Inquiry_Result(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	char szComPort[4 + 1] = {0};

	/* 不是ECR連線就跳走 */
	if (pobTran->uszECRBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	/* 不是兩段式交易，不用回傳查詢*/
	if (pobTran->uszCardInquiryFirstBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
		memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
		memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
		memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_ECR_Send_Transaction_Result(pobTran);
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		inRetVal = inUSB_ECR_Send_Transaction_Result(pobTran);
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		inRetVal = inBaseUSB_ECR_Send_Transaction_Result(pobTran);
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = inWiFi_ECR_Send_Result(pobTran);
	} else
	{
		inRetVal = VS_SUCCESS;
	}

	return (inRetVal);
}

/*
Function        :inECR_SendError
Date&Time       :2017/6/1 下午 6:17
Describe        :送錯誤訊息給ECR
 */
int inECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	int inRetVal = VS_ERROR;
	char szComPort[4 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
		memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
		memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
		memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_ECR_SendError(pobTran, inErrorType);
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		inRetVal = inUSB_ECR_SendError(pobTran, inErrorType);
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		inRetVal = inBaseUSB_ECR_SendError(pobTran, inErrorType);
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = inWiFi_ECR_Send_Error(pobTran, inErrorType);
	} else
	{
		inRetVal = VS_SUCCESS;
	}

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inECR_DeInitial
Date&Time       :2018/5/22 上午 10:30
Describe        :反初始化
 */
int inECR_DeInitial(void)
{
	int inRetVal = VS_ERROR;
	char szTMSOK[2 + 1];
	char szECREnable[2 + 1];
	char szComPort[4 + 1];

	/* 檢查是否做過【參數下載】 */
	memset(szTMSOK, 0x00, sizeof (szTMSOK));
	inGetTMSOK(szTMSOK);
	if (szTMSOK[0] != 'Y')
	{
//		inDISP_Msg_BMP(_ERR_TMS_DWL_, _COORDINATE_Y_LINE_8X16_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		return (VS_SUCCESS);
	}

	/* 檢查是否有支援收銀機連線 */
	memset(szECREnable, 0x00, sizeof (szECREnable));
	inGetECREnable(szECREnable);
	if (szECREnable[0] != 'Y')
	{
		return (VS_SUCCESS);
	}

	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
		memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
		memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
		memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_ECR_DeInitial();
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		inRetVal = inUSB_ECR_DeInitial();
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		inRetVal = VS_ERROR;
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = VS_ERROR;
	} else
	{
		inRetVal = VS_ERROR;
	}

	/* 根據ECRVersion來決定，COMPORT的設定 */
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_ClearAll();
		inDISP_Msg_BMP(_ERR_INIT_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "RS232", _LINE_8_5_);
	} else
	{
		/* 代表設定完成 */
		gsrECROb.srSetting.uszSettingOK = VS_FALSE;
	}

	return (VS_SUCCESS);
}

/*
Function        :inECR_Data_Send
Date&Time       :2017/11/15 下午 5:43
Describe        :分流
 */
int inECR_Data_Send(unsigned char *uszSendBuff, unsigned short usSendSize)
{
	int inRetVal = VS_ERROR;
	char szComPort[4 + 1];

	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
		memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
		memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
		memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_Data_Send(gsrECROb.srSetting.uszComPort, uszSendBuff, usSendSize);
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		inRetVal = inUSB_Data_Send(uszSendBuff, usSendSize);
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		inRetVal = inBaseUSB_Data_Send(uszSendBuff, usSendSize);
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = inETHERNET_Send_Data_By_Native(ginECR_ResponseFd, uszSendBuff, usSendSize);
	} else
	{
		inRetVal = VS_SUCCESS;
	}

	return (inRetVal);
}

/*
Function        :inECR_Data_Receive
Date&Time       :2017/11/15 下午 3:51
Describe        :底層function分流
 */
int inECR_Data_Receive(unsigned char *uszReceBuff, unsigned short *usReceSize)
{
	int inRetVal = VS_ERROR;
	char szComPort[4 + 1];

	memset(szComPort, 0x00, sizeof (szComPort));
	inGetECRComPort(szComPort);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag("-->Comport [%s] Receive Len [%d] gsrComPort [%d]", szComPort, *usReceSize, gsrECROb.srSetting.uszComPort);

	if (memcmp(szComPort, "COM1", strlen("COM1")) == 0 ||
		memcmp(szComPort, "COM2", strlen("COM2")) == 0 ||
		memcmp(szComPort, "COM3", strlen("COM3")) == 0 ||
		memcmp(szComPort, "COM4", strlen("COM4")) == 0)
	{
		inRetVal = inRS232_Data_Receive(gsrECROb.srSetting.uszComPort, uszReceBuff, usReceSize);
	} else if (memcmp(szComPort, "USB1", strlen("USB1")) == 0)
	{
		inRetVal = inUSB_Data_Receive(uszReceBuff, usReceSize);
	} else if (memcmp(szComPort, "USB2", strlen("USB2")) == 0)
	{
		inRetVal = inBaseUSB_Data_Receive(uszReceBuff, usReceSize);
	} else if (memcmp(szComPort, "WIFI", strlen("WIFI")) == 0)
	{
		inRetVal = inETHERNET_Receive_Data_By_Native(ginECR_ResponseFd, uszReceBuff, usReceSize);
	} else
	{
		inRetVal = VS_SUCCESS;
	}

	inDISP_LogPrintfWithFlag("-->RetVal [%d]  AftRec Size [%d] ", inRetVal, *usReceSize);
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
	return (inRetVal);
}

/*
Function        :inECR_Send_ACKorNAK
Date&Time       :2017/11/15 下午 6:11
Describe        :送ACK OR NAK
 */
int inECR_Send_ACKorNAK(ECR_TABLE * srECROb, int inAckorNak)
{
	unsigned char uszSendBuffer[2 + 1];
	unsigned short usRetVal;

	memset(uszSendBuffer, 0x00, sizeof (uszSendBuffer));

	/* 檢查port是否已經準備好要送資料 */
	while (inECR_Send_Check() != VS_SUCCESS)
	{
		/* 等TxReady*/
	};

	if (inAckorNak == _ACK_)
	{
		/* 成功，回傳ACK */
		uszSendBuffer[0] = _ACK_;
		uszSendBuffer[1] = _ACK_;

		usRetVal = inECR_Data_Send(uszSendBuffer, 2);

		if (usRetVal != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag(" Send Ack_Ack Data *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		} else
		{
			inDISP_LogPrintfWithFlag(" Send Ack_Ack Data Success Line[%d]", __LINE__);
		}

	}
	else if (inAckorNak == _NAK_)
	{
		/* 失敗，回傳NAK */
		uszSendBuffer[0] = _NAK_;
		uszSendBuffer[1] = _NAK_;

		usRetVal = inECR_Data_Send(uszSendBuffer, 2);

		if (usRetVal != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag(" Send Nak_Nak Data *Error* Line[%d]", __LINE__);
			return (VS_ERROR);
		} else
		{
			inDISP_LogPrintfWithFlag(" Send Nak_Nak Data Success Line[%d]", __LINE__);
		}

	} else
	{
		/* 傳入錯誤的參數 */
		inDISP_LogPrintfWithFlag(" Can't Send Neither ACK or NAK *Error* Line[%d]", __LINE__);
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inECR_Receive_ACKandNAK
Date&Time       :2017/11/15 下午 5:50
Describe        :
 */
int inECR_Receive_ACKandNAK(ECR_TABLE * srECROb)
{
	int inRetVal;
	char szDebugMsg[100 + 1];
	unsigned char uszBuf[_ECR_BUFF_SIZE_];
	unsigned short usTwoSize = 2;
	unsigned short usReceiveLen = 0;

	/* 設定Timeout */
	inRetVal = inDISP_Timer_Start(_TIMER_NEXSYS_2_, _ECR_RECEIVE_ACK_TIMEOUT_);

	while (1)
	{
		memset(uszBuf, 0x00, sizeof (uszBuf));
		/* 當COMPort中有東西就開始分析 */
		while (usReceiveLen == 0)
		{
			inECR_Receive_Check(&usReceiveLen);

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
			inRetVal = inECR_Data_Receive(uszBuf, &usTwoSize);

			if (inRetVal == VS_SUCCESS)
			{
				/* buffer讀出兩個byte，長度減二 */
				usReceiveLen -= 2;

				/* 判斷收到資料是否為ACK */
				if (uszBuf[0] == _ACK_ && uszBuf[1] == _ACK_)
				{
					inDISP_LogPrintfWithFlag(" Receive Ack_Ack Data Finish Line[%d]", __LINE__);
					return (_ACK_);
				}					/* 判斷收到資料是否為NAK */
				else if (uszBuf[0] == _NAK_ && uszBuf[1] == _NAK_)
				{
					inDISP_LogPrintfWithFlag(" Receive Nak_Nak Data Finish Line[%d]", __LINE__);
					return (_NAK_);
				} else
				{
					
					inDISP_LogPrintfWithFlag(" Receive Not Ack Neither NAK Line[%d]", __LINE__);
					inDISP_LogPrintfWithFlag(" uszBuf[0] (%02x) uszBuf[1] (%02x) Line[%d]", uszBuf[0], uszBuf[1], __LINE__);
					continue;
				}

			}/*  */

		}/* while (usReceiveLen > 0) (有收到資料) */

	}/* while(1)...... */

}

/*
Function        :inECR_Print_Receive_ISODeBug
Date&Time       :2016/11/1 上午 11:06
Describe        :印收到的ISODeBug
 */
int inECR_Print_Receive_ISODeBug(char *szDataBuffer, unsigned short usReceiveBufferSize, int inDataSize)
{
	int i;
	int inPrintPosition, inLeftTag, inISOPositionInLine, inISOPosition; /* ISODebug使用 */
	char szDebugMsg[1024 + 1]; /* 會用來印400ECR，所以放大一點 */
	char szTemplate[100 + 1];
	unsigned char uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle srBhandle1;
	FONT_ATTRIB srFont_Attrib1;

	/* ISO example
	 * ===============(400)(標準收_400_BYTE)
		  1234567890 1234567890 1234567890
	  001 I      001 N  000001
	  031              00000000 5100
	  061
	  091
	  121
	  151
	  181
	  211
	  241
	  271
	  301
	  331
	  361
	  391
	==================(標準結束_400_BYTE)
	 */
	if (ginISODebug == VS_TRUE)
	{
		/* 是否有列印功能 */
		if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
		{

		} else
		{
			/* 初始化 */
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));

			memset(szTemplate, 0x00, sizeof (szTemplate));
			inGetECRVersion(szTemplate);
			if (memcmp(szTemplate, "0", 1) == 0)
			{
				sprintf(szDebugMsg, "===============(%03d)(標準收_%d_BYTE)", usReceiveBufferSize, inDataSize);
			}
			inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("      1234567890 1234567890 1234567890", _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			inISOPositionInLine = 0; /* 印到該行第幾個Byte */
			inLeftTag = 1; /* 最左邊的欄位，從1開始，每次加30 */

			/* 最左邊的欄位 */
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "  %03d  ", inLeftTag);

			inPrintPosition = 6; /* 在紙上要從哪裡開始印，因為前面要空LeftTag的位置 */

			for (inISOPosition = 0; inISOPosition < usReceiveBufferSize; inISOPosition++)
			{
				szDebugMsg[inPrintPosition++] = szDataBuffer[inISOPosition];
				inISOPositionInLine++; /* 電文的第幾個byte */

				/* 每10個一個空格 */
				if ((inISOPositionInLine % 10) == 0)
					szDebugMsg[inPrintPosition++] = 0x20;

				/* 每30個印出來，歸0並換行 */
				if ((inISOPositionInLine % 30) == 0)
				{
					inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
					inISOPositionInLine = 0;
					inLeftTag += 30;

					/* 最左邊的欄位 */
					memset(szDebugMsg, 0x00, sizeof (printf));
					sprintf(szDebugMsg, "  %03d  ", inLeftTag);
					inPrintPosition = 6;
				} else if (inISOPosition == (inDataSize - 1))
				{
					inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
					break;
				}
			}

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));

			memset(szTemplate, 0x00, sizeof (szTemplate));
			inGetECRVersion(szTemplate);
			if (memcmp(szTemplate, "0", 1) == 0)
			{
				sprintf(szDebugMsg, "==================(標準結束_%03d_BYTE)", inDataSize);
			}
			inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			for (i = 0; i < 8; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1);
		}

	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetECRVersion(szTemplate);
		if (memcmp(szTemplate, "0", 1) == 0)
		{
			sprintf(szDebugMsg, "===============(%03d)(標準收_%d_BYTE)", usReceiveBufferSize, inDataSize);
		}

		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf(" ===  1234567890 1234567890 1234567890");

		inISOPositionInLine = 0; /* 印到該行第幾個Byte */
		inLeftTag = 1; /* 最左邊的欄位，從1開始，每次加30 */

		/* 最左邊的欄位 */
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "  %03d  ", inLeftTag);

		inPrintPosition = 6; /* 在紙上要從哪裡開始印，因為前面要空LeftTag的位置 */

		for (inISOPosition = 0; inISOPosition < usReceiveBufferSize; inISOPosition++)
		{
			szDebugMsg[inPrintPosition++] = szDataBuffer[inISOPosition];
			inISOPositionInLine++; /* 電文的第幾個byte */

			/* 每10個一個空格 */
			if ((inISOPositionInLine % 10) == 0)
				szDebugMsg[inPrintPosition++] = 0x20;

			/* 每30個印出來，歸0並換行 */
			if ((inISOPositionInLine % 30) == 0)
			{
				inDISP_LogPrintf(szDebugMsg);
				inISOPositionInLine = 0;
				inLeftTag += 30;

				/* 最左邊的欄位 */
				memset(szDebugMsg, 0x00, sizeof (printf));
				sprintf(szDebugMsg, "  %03d  ", inLeftTag);
				inPrintPosition = 6;
			} else if (inISOPosition == (inDataSize - 1))
			{
				/* 把最後那一行印出來 */
				inDISP_LogPrintf(szDebugMsg);
				break;
			}
		}

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));

		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetECRVersion(szTemplate);
		if (memcmp(szTemplate, "0", 1) == 0)
		{
			sprintf(szDebugMsg, "==================(標準結束_%03d_BYTE)", inDataSize);
		}
		inDISP_LogPrintf(szDebugMsg);
	}

	return (VS_SUCCESS);
}

/*
Function        :inECR_Print_Send_ISODeBug
Date&Time       :2016/11/1 上午 11:06
Describe        :印送出的ISODeBug
 */
int inECR_Print_Send_ISODeBug(char *szDataBuffer, unsigned short usReceiveBufferSize, int inDataSize)
{
	int i;
	int inPrintPosition, inLeftTag, inISOPositionInLine, inISOPosition; /* ISODebug使用 */
	char szDebugMsg[1024 + 1]; /* 會用來印400ECR，所以放大一點 */
	char szTemplate[100 + 1];
	unsigned char uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle srBhandle1;
	FONT_ATTRIB srFont_Attrib1;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	/* ISO example
	 * ===============(400)(標準送_400_BYTE)
		  1234567890 1234567890 1234567890
	  001 I      001 N  000001
	  031              00000000 5100
	  061
	  091
	  121
	  151
	  181
	  211
	  241
	  271
	  301
	  331
	  361
	  391
	==================(標準結束_400_BYTE)
	 */
	if (ginISODebug == VS_TRUE)
	{
		/* 是否有列印功能 */
		if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
		{
			return (VS_SUCCESS);
		} else
		{
			/* 初始化 */
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));

			memset(szTemplate, 0x00, sizeof (szTemplate));
			inGetECRVersion(szTemplate);
			if (memcmp(szTemplate, "0", 1) == 0)
			{
				sprintf(szDebugMsg, "===============(%03d)(標準送_%d_BYTE)", usReceiveBufferSize, inDataSize);
			}
			inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("      1234567890 1234567890 1234567890", _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			inISOPositionInLine = 0; /* 印到該行第幾個Byte */
			inLeftTag = 1; /* 最左邊的欄位，從1開始，每次加30 */

			/* 最左邊的欄位 */
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "  %03d  ", inLeftTag);

			inPrintPosition = 6; /* 在紙上要從哪裡開始印，因為前面要空LeftTag的位置 */

			for (inISOPosition = 0; inISOPosition < usReceiveBufferSize; inISOPosition++)
			{
				/* 0x00 印不出來改0x20 */
				if (szDataBuffer[inISOPosition] == 0x00)
				{
					szDebugMsg[inPrintPosition++] = 0x20;
				} else
				{
					szDebugMsg[inPrintPosition++] = szDataBuffer[inISOPosition];
				}
				inISOPositionInLine++; /* 電文的第幾個byte */

				/* 每10個一個空格 */
				if ((inISOPositionInLine % 10) == 0)
					szDebugMsg[inPrintPosition++] = 0x20;

				/* 每30個印出來，歸0並換行 */
				if ((inISOPositionInLine % 30) == 0)
				{
					inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
					inISOPositionInLine = 0;
					inLeftTag += 30;

					/* 最左邊的欄位 */
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "  %03d  ", inLeftTag);
					inPrintPosition = 6;
				} else if (inISOPosition == (usReceiveBufferSize - 1))
				{
					inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
					break;
				}

			}


			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));

			memset(szTemplate, 0x00, sizeof (szTemplate));
			inGetECRVersion(szTemplate);
			if (memcmp(szTemplate, "0", 1) == 0)
			{
				sprintf(szDebugMsg, "==================(標準結束_%03d_BYTE)", inDataSize);
			}
			inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			for (i = 0; i < 8; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1);
		}
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetECRVersion(szTemplate);
		if (memcmp(szTemplate, "0", 1) == 0)
		{
			sprintf(szDebugMsg, "===============(%03d)(標準送_%d_BYTE)", usReceiveBufferSize, inDataSize);
		}
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("      1234567890 1234567890 1234567890");

		inISOPositionInLine = 0; /* 印到該行第幾個Byte */
		inLeftTag = 1; /* 最左邊的欄位，從1開始，每次加30 */

		/* 最左邊的欄位 */
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "  %03d  ", inLeftTag);

		inPrintPosition = 6; /* 在紙上要從哪裡開始印，因為前面要空LeftTag的位置 */

		for (inISOPosition = 0; inISOPosition < usReceiveBufferSize; inISOPosition++)
		{
			/* 0x00 印不出來改0x20 */
			if (szDataBuffer[inISOPosition] == 0x00)
			{
				szDebugMsg[inPrintPosition++] = 0x20;
			} else
			{
				szDebugMsg[inPrintPosition++] = szDataBuffer[inISOPosition];
			}
			inISOPositionInLine++; /* 電文的第幾個byte */

			/* 每10個一個空格 */
			if ((inISOPositionInLine % 10) == 0)
				szDebugMsg[inPrintPosition++] = 0x20;

			/* 每30個印出來，歸0並換行 */
			if ((inISOPositionInLine % 30) == 0)
			{
				inDISP_LogPrintf(szDebugMsg);
				inISOPositionInLine = 0;
				inLeftTag += 30;

				/* 最左邊的欄位 */
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "  %03d  ", inLeftTag);
				inPrintPosition = 6;
			} else if (inISOPosition == (usReceiveBufferSize - 1))
			{
				/* 把最後那一行印出來 */
				inDISP_LogPrintf(szDebugMsg);
				break;
			}
		}


		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));

		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetECRVersion(szTemplate);
		if (memcmp(szTemplate, "0", 1) == 0)
		{
			sprintf(szDebugMsg, "==================(標準結束_%03d_BYTE)", inDataSize);
		}
		inDISP_LogPrintf(szDebugMsg);
	}

	return (VS_SUCCESS);
}

/*
Function        :inECRCardNoTruncateDecision
Date&Time       :2017/5/22 下午 4:33
Describe        :判斷ECR交易回送資料給POS機時,卡號是否要做掩飾
 *		1. VS_TRUE -> 卡號要做掩飾
		2. VS_FALSE -> 卡號不做掩飾
 *		看TMS規格應該不會有CardBin重疊的現象
 */
int inECR_CardNoTruncateDecision(TRANSACTION_OBJECT * pobTran)
{
	int inRetVal = VS_ERROR;
	int i = 0;
	int inLowBinLen = 0, inHighBinLen = 0;
	int inPANLen = 0;
	long lnCampaignAmount = 0;
	char szFuncEnable[2 + 1];
	char szLowBinRange[2 + 1];
	char szHighBinRange[2 + 1];
	char szTempCampaignNumber[2 + 1];
	char szTemplate[12 + 1];
	char szStartDate[8 + 1], szEndDate[8 + 1];
	char szNowDate[8 + 1];
	char szDebugMsg[100 + 1];
	unsigned char uszInCardBin = VS_ERROR; /* 這表示在卡號內 */
	RTC_NEXSYS srRTC;


	inGetECRCardNoTruncateEnable(szFuncEnable);
	/* 如果TMS 設定 回傳卡號 需要掩飾,才有必要比對活動代碼,否則不需要比對,因為已經是明碼回傳卡號 */
	if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
	{
		return (VS_ERROR);
	} else
	{
		/* CFGT沒開特殊卡別參數檔 */
		memset(szFuncEnable, 0x00, sizeof (szFuncEnable));
		inGetSpecialCardRangeEnable(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
		{
			return (VS_ERROR);
		}

		inPANLen = strlen(pobTran->srBRec.szPAN);
		if (inPANLen < 0)
		{
			/* debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "PANLength: %d < 0", inPANLen);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 顯示卡號錯誤 */
			inDISP_Msg_BMP(_ERR_CARD_NO_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 180, "", 0);

			return (VS_ERROR);
		}

		for (i = 0;; i++)
		{
			if (inLoadSCDTRec(i) < 0)
				break;

			memset(szLowBinRange, 0x00, sizeof (szLowBinRange));
			memset(szHighBinRange, 0x00, sizeof (szHighBinRange));
			inGetLowBinRange(szLowBinRange);
			inLowBinLen = strlen(szLowBinRange);
			inGetHighBinRange(szHighBinRange);
			inHighBinLen = strlen(szHighBinRange);

			memset(szTempCampaignNumber, 0x00, sizeof (szTempCampaignNumber));
			inGetCampaignNumber(szTempCampaignNumber);
			/* 如果不是要判別的活動代碼，就跳過，這裡的設計原理一次只確認一種活動代碼(為了模組化) */
			if (memcmp(szTempCampaignNumber, "01", 2) != 0)
			{
				continue;
			}

			/* CUP卡看CardBin為9999999999就好 */
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)) == 0)
			{
				/* 判斷卡號 位在哪個Bin Range(如果卡號大於該卡別最低卡號，而且小於最高卡號) */
				if ((memcmp(szLowBinRange, "9999999999", 10) >= 0) &&
					(memcmp(szHighBinRange, "9999999999", 10) <= 0))
				{
					uszInCardBin = VS_TRUE;
					break;
				}
			} else
			{
				if ((memcmp(pobTran->srBRec.szPAN, szLowBinRange, min(inPANLen, inLowBinLen)) >= 0) &&
						(memcmp(pobTran->srBRec.szPAN, szHighBinRange, min(inPANLen, inHighBinLen)) <= 0))
				{
					uszInCardBin = VS_TRUE;
					break;
				}
			}
		}

		/* SCDT內沒有該卡別 */
		if (uszInCardBin != VS_TRUE)
		{
			return (VS_ERROR);
		} else
		{
			/* 檢查金額 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			inGetCampaignAmount(szTemplate);
			/* 小數點下兩位不記*/
			szTemplate[10] = 0x00;
			szTemplate[11] = 0x00;
			lnCampaignAmount = atol(szTemplate);

			if (pobTran->srBRec.lnTxnAmount <= lnCampaignAmount)
			{
				/* 取得活動起迄日 */
				/* 活動起日 */
				memset(szStartDate, 0x00, sizeof (szStartDate));
				inGetCampaignStartDate(szStartDate);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "CampaignStartDate = %s", szStartDate);
					inDISP_LogPrintf(szDebugMsg);
				}

				/* 活動迄日 */
				memset(szEndDate, 0x00, sizeof (szEndDate));
				inGetCampaignEndDate(szEndDate);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "CampaignEndDate = %s", szEndDate);
					inDISP_LogPrintf(szDebugMsg);
				}

				/* 現在日期 */
				memset(&srRTC, 0x00, sizeof (RTC_NEXSYS));
				inFunc_GetSystemDateAndTime(&srRTC);
				memset(szNowDate, 0x00, sizeof (szNowDate));
				sprintf(szNowDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "NowDate = %s", szNowDate);
					inDISP_LogPrintf(szDebugMsg);
				}

				if (inFunc_SunDay_Sum_Check_In_Range(szNowDate, szStartDate, szEndDate) == VS_SUCCESS)
				{
					inRetVal = VS_SUCCESS;
				} else
				{
					inRetVal = VS_ERROR;
				}
			}
		}
	}

	return (inRetVal);
}

/*
Function        :inECRCardNoTruncateDecision_HG
Date&Time       :2017/5/22 下午 4:33
Describe        :判斷ECR交易回送資料給POS機時,卡號是否要做掩飾
 *		1. VS_TRUE -> 卡號要做掩飾
		2. VS_FALSE -> 卡號不做掩飾
 *		看TMS規格應該不會有CardBin重疊的現象
 */
int inECR_CardNoTruncateDecision_HG(TRANSACTION_OBJECT * pobTran)
{
	int inRetVal = VS_ERROR;
	int i = 0;
	int inLowBinLen = 0, inHighBinLen = 0;
	int inPANLen = 0;
	long lnCampaignAmount = 0;
	char szFuncEnable[2 + 1];
	char szLowBinRange[2 + 1];
	char szHighBinRange[2 + 1];
	char szTempCampaignNumber[2 + 1];
	char szTemplate[12 + 1];
	char szStartDate[8 + 1], szEndDate[8 + 1];
	char szNowDate[8 + 1];
	char szDebugMsg[100 + 1];
	unsigned char uszInCardBin = VS_ERROR; /* 這表示在卡號內 */
	RTC_NEXSYS srRTC;


	inGetECRCardNoTruncateEnable(szFuncEnable);
	/* 如果TMS 設定 回傳卡號 需要掩飾,才有必要比對活動代碼,否則不需要比對,因為已經是明碼回傳卡號 */
	if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
	{
		return (VS_ERROR);
	} else
	{
		/* CFGT沒開特殊卡別參數檔 */
		memset(szFuncEnable, 0x00, sizeof (szFuncEnable));
		inGetSpecialCardRangeEnable(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
		{
			return (VS_ERROR);
		}

		inPANLen = strlen(pobTran->srBRec.szHGPAN);
		if (inPANLen < 0)
		{
			/* debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "PANLength: %d < 0", inPANLen);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 顯示卡號錯誤 */
			inDISP_Msg_BMP(_ERR_CARD_NO_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, 180, "", 0);

			return (VS_ERROR);
		}

		for (i = 0;; i++)
		{
			if (inLoadSCDTRec(i) < 0)
				break;

			memset(szLowBinRange, 0x00, sizeof (szLowBinRange));
			memset(szHighBinRange, 0x00, sizeof (szHighBinRange));
			inGetLowBinRange(szLowBinRange);
			inLowBinLen = strlen(szLowBinRange);
			inGetHighBinRange(szHighBinRange);
			inHighBinLen = strlen(szHighBinRange);

			memset(szTempCampaignNumber, 0x00, sizeof (szTempCampaignNumber));
			inGetCampaignNumber(szTempCampaignNumber);
			/* 如果不是要判別的活動代碼，就跳過，這裡的設計原理一次只確認一種活動代碼(為了模組化) */
			if (memcmp(szTempCampaignNumber, "01", 2) != 0)
			{
				continue;
			}

			/* CUP卡看CardBin為9999999999就好 */
			if (memcmp(pobTran->srBRec.szHGCardLabel, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)) == 0)
			{
				/* 判斷卡號 位在哪個Bin Range(如果卡號大於該卡別最低卡號，而且小於最高卡號) */
				if ((memcmp(szLowBinRange, "9999999999", 10) >= 0) &&
						(memcmp(szHighBinRange, "9999999999", 10) <= 0))
				{
					uszInCardBin = VS_TRUE;
					break;
				}
			} else
			{
				if ((memcmp(pobTran->srBRec.szHGPAN, szLowBinRange, min(inPANLen, inLowBinLen)) >= 0) &&
						(memcmp(pobTran->srBRec.szHGPAN, szHighBinRange, min(inPANLen, inHighBinLen)) <= 0))
				{
					uszInCardBin = VS_TRUE;
					break;
				}
			}
		}

		/* SCDT內沒有該卡別 */
		if (uszInCardBin != VS_TRUE)
		{
			return (VS_ERROR);
		} else
		{
			/* 檢查金額 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			inGetCampaignAmount(szTemplate);
			/* 小數點下兩位不記*/
			szTemplate[10] = 0x00;
			szTemplate[11] = 0x00;
			lnCampaignAmount = atol(szTemplate);

			if (pobTran->srBRec.lnTxnAmount <= lnCampaignAmount)
			{
				/* 取得活動起迄日 */
				/* 活動起日 */
				memset(szStartDate, 0x00, sizeof (szStartDate));
				inGetCampaignStartDate(szStartDate);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "CampaignStartDate = %s", szStartDate);
					inDISP_LogPrintf(szDebugMsg);
				}

				/* 活動迄日 */
				memset(szEndDate, 0x00, sizeof (szEndDate));
				inGetCampaignEndDate(szEndDate);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "CampaignEndDate = %s", szEndDate);
					inDISP_LogPrintf(szDebugMsg);
				}

				/* 現在日期 */
				memset(&srRTC, 0x00, sizeof (RTC_NEXSYS));
				inFunc_GetSystemDateAndTime(&srRTC);
				memset(szNowDate, 0x00, sizeof (szNowDate));
				sprintf(szNowDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "NowDate = %s", szNowDate);
					inDISP_LogPrintf(szDebugMsg);
				}

				if (inFunc_SunDay_Sum_Check_In_Range(szNowDate, szStartDate, szEndDate) == VS_SUCCESS)
				{
					inRetVal = VS_SUCCESS;
				} else
				{
					inRetVal = VS_ERROR;
				}
			}
		}
	}

	return (inRetVal);
}

/*
Function        :inECR_Receive
Date&Time       :2017/11/15 下午 3:14
Describe        :接收收銀機傳來的資料
 */
int inECR_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int i;
	int inRetVal;
	int inRetry = 0; /* 目前已重試次數 */
	int inRecMaxRetry = _ECR_RETRYTIMES_; /* 最大重試次數 */
	char szDebugMsg[1000 + 1]; /* DebugMsg */
	unsigned short usReceiveBufferSize; /* uszReceiveBuffer的長度，*/
	unsigned short usOneSize = 1; /* 一次只讀一個byte */
	unsigned short usReceiveLen = 0;
	unsigned char uszLRC; /* LRC的值 */
	unsigned char uszTempBuffer[1024 + 1]; /* 從 Buffer收到的片段 */
	unsigned char uszReceiveBuffer[_ECR_BUFF_SIZE_]; /* 包含STX 和 ETX的原始電文 */
	unsigned char uszSTX = VS_FALSE; /* 是否收到STX */
	unsigned char uszETX = VS_FALSE; /* 是否收到ETX */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inFunc_CalculateRunTimeGlobal_Start();

	/* 設定Timeout */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inDISP_Timer_Start(_ECR_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);

	/* 初始化放收到資料的陣列 */
	memset(uszReceiveBuffer, 0x00, sizeof (uszReceiveBuffer));
	usReceiveBufferSize = 0;

	while (1)
	{
		memset(uszTempBuffer, 0x00, sizeof (uszTempBuffer));
		uszLRC = 0;

		/* 超過重試次數 */
		if (inRetry > inRecMaxRetry)
		{
			inDISP_LogPrintfWithFlag(" Retry too many times! Line[%d]", __LINE__);
			return (VS_ERROR);
		}

		/* 當Comport中有東西就開始分析 */
		while (usReceiveLen == 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}

			inECR_Receive_Check(&usReceiveLen);
		}

		inDISP_LogPrintfWithFlag(" First ECR Check Len : [%d] Line[%d]", usReceiveLen, __LINE__);

		/* 如果沒收到STX就會進這隻，直到分析出STX */
		while (uszSTX == VS_FALSE && usReceiveLen > 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}

			/* 這邊一次只收一個byte */
			memset(uszTempBuffer, 0x00, 1);
			usOneSize = 1;
			inRetVal = inECR_Data_Receive(uszTempBuffer, &usOneSize);
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag("--> Line[%d] Size[%d]--------", __LINE__, usOneSize);
				
				/* buffer讀出一個byte，長度減一 */
				usReceiveLen--;

				if (uszTempBuffer[0] == _STX_)
				{
					/* 收到STX */
					uszSTX = VS_TRUE;
					inDISP_LogPrintfWithFlag("-->[%d] Receive STX  usRecLen[%d] ", __LINE__, usReceiveLen);
					inDISP_LogPrintfWithFlag("-->[%d] RecBufSize [%d] ", __LINE__, usReceiveBufferSize);

					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "Receive STX");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
					}

					memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], 1);
					usReceiveBufferSize++;

					inDISP_LogPrintfWithFlag("-->[%d] Aft Copy Data RecBufSize [%d] ", __LINE__, usReceiveBufferSize);
					inFunc_WatchRunTime();
					break;
				} else
				{
					/* 沒收到STX */
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Not Receive STX!");
						inDISP_LogPrintfWithFlag("--> Line [%d] Buf[%x]  ", __LINE__, uszTempBuffer[0]);
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						inDISP_LOGDisplay("Not Receive STX", _FONTSIZE_16X22_, _LINE_16_3_, VS_FALSE);
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszTempBuffer, usOneSize);
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_4_, VS_FALSE);
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
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}

			inDISP_LogPrintfWithFlag("-->[%d] Bef RunDataRece Reclen[%d]  ", __LINE__, usReceiveLen);

			usOneSize = 400;
			inRetVal = inECR_Data_Receive(&uszReceiveBuffer[usReceiveBufferSize], &usOneSize);

			if (inRetVal == VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag("-->[%d] Aft RunDataRece Success RecLen[%d]  ", __LINE__, usReceiveLen);
				inDISP_LogPrintfWithFlag("-->[%d] Aft RunDataRece usOneSize[%d]  ", __LINE__, usOneSize);
				inDISP_LogPrintfWithFlag("-->[%d] Aft RunDataRece inDataSize[%d]  ", __LINE__, inDataSize);

				/* buffer讀出一個byte，長度減一 */
				usReceiveLen -= usOneSize;
				usReceiveBufferSize += usOneSize;

				inDISP_LogPrintfWithFlag("-->[%d] Count  usReceiveLen[%d]  ", __LINE__, usReceiveLen);
				inDISP_LogPrintfWithFlag("-->[%d] Count  usReceiveBufferSize[%d]  ", __LINE__, usReceiveBufferSize);

				/* 當已收到401byte 確認下個byte是不是ETX */
				if (usReceiveBufferSize >= (inDataSize + 3) - 1)
				{
					if (uszReceiveBuffer[(inDataSize + 3) - 2] == _ETX_)
					{
						/* 是ETX */
						uszETX = VS_TRUE;
						if (ginDebug == VS_TRUE)
						{
							inDISP_LogPrintf("Receive ETX!");
						}
						if (ginDisplayDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "Receive ETX");
							inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
						}

						inFunc_WatchRunTime();

						if (usReceiveBufferSize < (inDataSize + 3))
						{
							/* 再收LRC */
							usOneSize = 1;
							inRetVal = inECR_Data_Receive(&uszReceiveBuffer[usReceiveBufferSize], &usOneSize);
							if (inRetVal == VS_SUCCESS)
							{
								inDISP_LogPrintfWithFlag("-->[%d] Aft Rec LRC Success RecLen[%d]  ", __LINE__, usReceiveLen);
								inDISP_LogPrintfWithFlag("-->[%d] Aft Rec LRC e usOneSize[%d]  ", __LINE__, usOneSize);

								usReceiveLen -= usOneSize;
								usReceiveBufferSize += usOneSize;

								inDISP_LogPrintfWithFlag("-->[%d] Aft Rec LRC Count  usReceiveLen[%d]  ", __LINE__, usReceiveLen);
								inDISP_LogPrintfWithFlag("-->[%d] Aft Rec LRC Count  usReceiveBufferSize[%d]  ", __LINE__, usReceiveBufferSize);
							} else
							{
								/* 沒收到LRC 資料長度有誤*/
							}
						}

						inFunc_WatchRunTime();
						break;
					} else
					{
						/* 不是是ETX */
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							inFunc_BCD_to_ASCII(szDebugMsg, uszTempBuffer, usOneSize);
							inDISP_LogPrintf(szDebugMsg);
							inDISP_LogPrintf("Not Receive ETX!");
						}
						if (ginDisplayDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "Not Receive ETX");
							inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
						}
					}
				}
			}

		}


		/* 有收到STX、ETX而且收到長度也對，就開始算LRC */
		if (uszSTX == VS_TRUE && uszETX == VS_TRUE)
		{
			/* 資料長度錯誤，清Buffer，送NAK，重收 */
			if (usReceiveBufferSize < inDataSize + 3)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "DataLength error!:%d Retry: %d", usReceiveBufferSize, inRetry);
					inDISP_LogPrintf(szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "DataLength error!:%d Retry: %d", usReceiveBufferSize, inRetry);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}

				/* 回傳NAK */
				inECR_Send_ACKorNAK(srECROb, _NAK_);
				/* retry次數+ 1 */
				inRetry++;

				/* 初始化資料 */
				memset(uszReceiveBuffer, 0x00, sizeof (uszReceiveBuffer));
				usReceiveBufferSize = 0;
				/* 理論上這個值應該也要重設為0 [SAM 181023] */
				usReceiveLen = 0;

				uszSTX = VS_FALSE;
				uszETX = VS_FALSE;

				continue;
			} else if (usReceiveBufferSize > inDataSize + 3)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "DataLength :%d", usReceiveBufferSize);
					inDISP_LogPrintf(szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "DataLength :%d", usReceiveBufferSize);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
			}

			/* 計算收到DATA的LRC （Stx Not Include）*/
			for (i = 1; i <= (usReceiveBufferSize - 2); i++)
			{
				uszLRC ^= uszReceiveBuffer[i];
			}

			if (uszReceiveBuffer[usReceiveBufferSize - 1] == uszLRC)
			{
				if (srECROb->srOptionalSetting.uszNotSendAck == VS_TRUE)
				{

				} else
				{
					/* 比對收到的LRC是否正確，若正確回傳ACK */
					inECR_Send_ACKorNAK(srECROb, _ACK_);
				}

				inFunc_WatchRunTime();
				break;
			} else
			{
				/* 比對失敗 */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "LRC error! Retry: %d", inRetry);
					inDISP_LogPrintf(szDebugMsg);

					inDISP_LogPrintf("CountLRC");
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%X", uszLRC);
					inDISP_LogPrintf(szDebugMsg);

					inDISP_LogPrintf("DataSendLRC");
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(&szDebugMsg[0], "%X", uszReceiveBuffer[usReceiveBufferSize - 1]);
					inDISP_LogPrintf(szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "LRC error! Retry: %d", inRetry);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}

				/* 若錯誤回傳NAK */
				inECR_Send_ACKorNAK(srECROb, _NAK_);
				/* retry次數+ 1 */
				inRetry++;

				/* 初始化資料 */
				memset(uszReceiveBuffer, 0x00, sizeof (uszReceiveBuffer));
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

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        : inECR_ReceiveWithoutCheckLenth
Date&Time   : 20190517
Describe        : 接收收銀機傳來的資料,只檢查不要超過最大暫存的Buffer
 */
int inECR_ReceiveWithoutCheckLenth(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int inBreak, inRetVal, i;
	int inRetry = 0; /* 目前已重試次數 */
	int inRecMaxRetry = _ECR_RETRYTIMES_; /* 最大重試次數 */
	char szDebugMsg[1000 + 1]; /* DebugMsg */
	unsigned short usReceiveBufferSize; /* 實際從 Buffer 讀出來的 uszReceiveBuffer的長度 */
	unsigned short usRealUseCopyLen, usUseCopyLen; /* 用來暫存算出來到 ETX 的ReceiveBuffer長度 */

	unsigned short usOneSize = 1; /* 一次只讀一個byte */
	unsigned short usReceiveLen = 0;
	unsigned char uszLRC; /* LRC的值 */
	unsigned char uszTempBuffer[1024 + 1]; /* 從 Buffer收到的片段 */
	unsigned char uszReceiveBuffer[_ECR_BUFF_SIZE_]; /* 包含STX 和 ETX的原始電文 */
	unsigned char uszSTX = VS_FALSE; /* 是否收到STX */
	unsigned char uszETX = VS_FALSE; /* 是否收到ETX */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);
	inDISP_LogPrintfWithFlag("  TimeOut [%d] ", srECROb->srSetting.inTimeout);

	inFunc_CalculateRunTimeGlobal_Start();

	/* 設定Timeout */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inDISP_Timer_Start(_ECR_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);

	/* 初始化放收到資料的陣列 */
	memset(uszReceiveBuffer, 0x00, sizeof (uszReceiveBuffer));
	usReceiveBufferSize = 0;
	usRealUseCopyLen = 0;

	while (1)
	{
		memset(uszTempBuffer, 0x00, sizeof (uszTempBuffer));
		uszLRC = 0;

		/* 超過重試次數 */
		if (inRetry > inRecMaxRetry)
		{
			inDISP_LogPrintfWithFlag("  Retry too many times!! ");
			return (VS_ERROR);
		}

		/* 當Comport中有東西就開始分析 */
		while (usReceiveLen == 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag("  Run Get Receive Len Time Out!! ");
				return (VS_TIMEOUT);
			}

			/* 讀出BUFFER中的資料長度 */
			inECR_Receive_Check(&usReceiveLen);
		}


		inDISP_LogPrintfWithFlag(" First ECR Check Len : %d", usReceiveLen);

		/* 如果沒收到STX就會進這隻，直到分析出STX */
		while (uszSTX == VS_FALSE && usReceiveLen > 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}

			/* 這邊一次只收一個byte */
			memset(uszTempBuffer, 0x00, 1);
			usOneSize = 1;
			inRetVal = inECR_Data_Receive(uszTempBuffer, &usOneSize);

			if (inRetVal == VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag("-->Line[%d] Size[%d] Buf[0][%x]--------", __LINE__, inRetVal, usOneSize, uszTempBuffer[0]);

				/* buffer讀出 [STX] 一個byte，長度減一 */
				/* 從BUFFER讀出資料，每讀一個BYTE長度就必需減一，直到讀出[STX]為止 */
				usReceiveLen--;

				if (uszTempBuffer[0] == _STX_)
				{
					/* 收到STX */
					uszSTX = VS_TRUE;
					inDISP_LogPrintfWithFlag("-->Line[%d] Receive STX usRecLen[%d] ", __LINE__, usReceiveLen);
					inDISP_LogPrintfWithFlag("-->Line[%d] RecBufSize [%d] ", __LINE__, usReceiveBufferSize);

					memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], 1);
					usReceiveBufferSize++;

					inDISP_LogPrintfWithFlag("-->Line [%d] Aft Copy Data RecBufSize [%d] ", __LINE__, usReceiveBufferSize);
					inFunc_WatchRunTime();
					break;
				} else
				{
					/* 沒收到STX */
					inDISP_LogPrintfWithFlag("  Not Receive STX!");
					inDISP_LogPrintfWithFlag("--> Line[%d] Buf[%x]  ", __LINE__, uszTempBuffer[0]);

					if (ginDisplayDebug == VS_TRUE)
					{
						inDISP_LOGDisplay("Not Receive STX", _FONTSIZE_16X22_, _LINE_16_3_, VS_FALSE);
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszTempBuffer, usOneSize);
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_4_, VS_FALSE);
					}

					/* 繼續收 */
					continue;
				}

			}

		}

		/* 如果有收到STX，沒收到ETX就會進這隻，直到分析出ETX */
		//while (uszSTX == VS_TRUE && uszETX == VS_FALSE && usReceiveLen > 0)
		while (uszSTX == VS_TRUE && uszETX == VS_FALSE)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}

			inDISP_LogPrintfWithFlag("-->[%d] Bef RunDataRece Reclen[%d]  ", __LINE__, usReceiveLen);

			/* 設定預定最大讀取長度， inECR_Data_Receive 後 會改為實際讀取到的資料長度 */
			usOneSize = 400;

			/* 0.1秒收一次,不需要收太快 */
			inDISP_Wait(100);

			inRetVal = inECR_Data_Receive(&uszReceiveBuffer[usReceiveBufferSize], &usOneSize);

			if (inRetVal == VS_SUCCESS)
			{
				inDISP_LogPrintfWithFlag("-->Line[%d] AftRec  usReceiveLen[%d] usOneSize[%d] inDataSize[%d]  ", __LINE__, usReceiveLen, usOneSize, inDataSize);

				usReceiveBufferSize += usOneSize;

				inDISP_LogPrintfWithFlag("-->Line[%d] AftCount usReceiveLen[%d]  usReceiveBufferSize[%d]  ", __LINE__, usReceiveLen, usReceiveBufferSize);

				inBreak = VS_FALSE;

				/* 設定暫存所要使用的目前抓取到的BUFFER長度   */
				usUseCopyLen = usReceiveBufferSize;

				/* 只檢查讀取到的長度，由最後一個BYTE檢查起 */
				do
				{
					inDISP_LogPrintfWithFlag(" --> usUseCopyLen [%d] BufferData [%x] usOneSize[%x] ",
							usUseCopyLen, uszReceiveBuffer[usUseCopyLen], usOneSize);

					/* 查找BUFFER 中是否有 ETX  */
					if (uszReceiveBuffer[usUseCopyLen] == _ETX_)
					{
						inBreak = VS_TRUE;
						/* 是ETX */
						uszETX = VS_TRUE;
						inFunc_WatchRunTime();
						/* 設定抓取到STX - ETX 的資料長度，以便做計算 */
						usRealUseCopyLen = usUseCopyLen;
					}


					usOneSize--;
					usUseCopyLen--;

				} while (usOneSize > 0 && inBreak != VS_TRUE);

			} else
			{
				inDISP_LogPrintfWithFlag(" -->Receive Err  [%d] ", inRetVal);
				/* 若錯誤回傳NAK */
				inECR_Send_ACKorNAK(srECROb, _NAK_);
				/* retry次數+ 1 */

				inRetry++;
				/* 初始化資料 */
				memset(uszReceiveBuffer, 0x00, sizeof (uszReceiveBuffer));
				usReceiveBufferSize = 0;
				uszSTX = VS_FALSE;
				uszETX = VS_FALSE;
			}
		}

		inDISP_LogPrintfWithFlag(" -->  Bef Check STX ");

		/* 有收到STX、ETX而且收到長度也對，就開始算LRC */
		if (uszSTX == VS_TRUE && uszETX == VS_TRUE)
		{
			inDISP_LogPrintfWithFlag(" -->  usRealUseCopyLen[%d]", usRealUseCopyLen);

			/* 計算收到DATA的LRC （Stx Not Include）*/
			for (i = 1; i <= usRealUseCopyLen; i++)
			{
				uszLRC ^= uszReceiveBuffer[i];
			}

			if (uszReceiveBuffer[usRealUseCopyLen + 1 ] == uszLRC)
			{
				if (srECROb->srOptionalSetting.uszNotSendAck == VS_TRUE)
				{

				} else
				{
					/* 比對收到的LRC是否正確，若正確回傳ACK */
					inECR_Send_ACKorNAK(srECROb, _ACK_);
				}

				inFunc_WatchRunTime();
				break;
			} else
			{
				/* 比對失敗 */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "LRC error! Retry: %d", inRetry);
					inDISP_LogPrintf(szDebugMsg);

					inDISP_LogPrintf("CountLRC");
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%X", uszLRC);
					inDISP_LogPrintf(szDebugMsg);

					inDISP_LogPrintf("DataSendLRC");
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(&szDebugMsg[0], "%X", uszReceiveBuffer[usReceiveBufferSize - 1]);
					inDISP_LogPrintf(szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "LRC error! Retry: %d", inRetry);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}

				/* 若錯誤回傳NAK */
				inECR_Send_ACKorNAK(srECROb, _NAK_);
				/* retry次數+ 1 */
				inRetry++;

				/* 初始化資料 */
				memset(uszReceiveBuffer, 0x00, sizeof (uszReceiveBuffer));
				usReceiveBufferSize = 0;
				uszSTX = VS_FALSE;
				uszETX = VS_FALSE;

				continue;
			}

		}/* 算LRC END */

	}/* Receive END */

	/* 去除STX、ETX、LRC，把資料放到szrRealReceBuffer */
	memcpy(szDataBuffer, &uszReceiveBuffer[1], (usRealUseCopyLen - 1));

	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Receive_ISODeBug(szDataBuffer, usReceiveBufferSize - 3, (usRealUseCopyLen - 2));

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

/*
Function        :inECR_Not_Check_Sizes_Receive
Date&Time       :2017/11/16 上午 11:45
Describe        :
 */
int inECR_Not_Check_Sizes_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int i;
	int inRetVal;
	int inRetry = 0; /* 目前已重試次數 */
	int inRecMaxRetry = _ECR_RETRYTIMES_; /* 最大重試次數 */
	char szDebugMsg[100 + 1]; /* DebugMsg */
	unsigned short usReceiveBufferSize; /* uszReceiveBuffer的長度，*/
	unsigned short usOneSize = 1; /* 一次只讀一個byte */
	unsigned short usReceiveLen = 0;
	unsigned char uszLRC; /* LRC的值 */
	unsigned char uszTempBuffer[1024 + 1]; /* 從 Buffer收到的片段 */
	unsigned char uszReceiveBuffer[_ECR_BUFF_SIZE_]; /* 包含STX 和 ETX的原始電文 */
	unsigned char uszSTX = VS_FALSE; /* 是否收到STX */
	unsigned char uszETX = VS_FALSE; /* 是否收到ETX */

	/* 設定Timeout */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inDISP_Timer_Start(_ECR_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);

	/* 初始化放收到資料的陣列 */
	memset(uszReceiveBuffer, 0x00, sizeof (uszReceiveBuffer));
	usReceiveBufferSize = 0;

	while (1)
	{
		memset(uszTempBuffer, 0x00, sizeof (uszTempBuffer));
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

		/* 當Comport中有東西就開始分析 */
		while (usReceiveLen == 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}

			inECR_Receive_Check(&usReceiveLen);
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Len : %d", usReceiveLen);
			inDISP_LogPrintf(szDebugMsg);
		}


		/* 如果沒收到STX就會進這隻，直到分析出STX */
		while (uszSTX == VS_FALSE && usReceiveLen > 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}

			/* 這邊一次只收一個byte */
			memset(uszTempBuffer, 0x00, 1);
			usOneSize = 1;
			inRetVal = inECR_Data_Receive(uszTempBuffer, &usOneSize);
			if (inRetVal == VS_SUCCESS)
			{
				/* buffer讀出一個byte，長度減一 */
				usReceiveLen--;

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
				} else
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
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}

			usOneSize = 400;
			inRetVal = inECR_Data_Receive(&uszReceiveBuffer[usReceiveBufferSize], &usOneSize);

			if (inRetVal == VS_SUCCESS)
			{

				/* buffer讀出一個byte，長度減一 */
				usReceiveLen -= usOneSize;

				if (uszReceiveBuffer[usReceiveBufferSize] == _ETX_)
				{
					/* 是ETX */
					uszETX = VS_TRUE;
					usReceiveBufferSize++;
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Receive ETX!");
					}

					/* 再收LRC */
					usOneSize = 1;
					inRetVal = inECR_Data_Receive(&uszReceiveBuffer[usReceiveBufferSize], &usOneSize);
					if (inRetVal == VS_SUCCESS)
					{
						usReceiveLen--;
						usReceiveBufferSize++;
					} else
					{
						/* 沒收到LRC 資料長度有誤*/
					}
					break;
				} else
				{
					/* 不是是ETX */
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszTempBuffer, usOneSize);
						inDISP_LogPrintf(szDebugMsg);
						inDISP_LogPrintf("Not Receive ETX!");
					}
					usReceiveBufferSize++;
				}

			}

		}


		/* 有收到STX、ETX而且收到長度也對，就開始算LRC */
		if (uszSTX == VS_TRUE && uszETX == VS_TRUE)
		{
			/* 計算收到DATA的LRC （Stx Not Include）*/
			for (i = 1; i <= (usReceiveBufferSize - 2); i++)
			{
				uszLRC ^= uszReceiveBuffer[i];
			}

			if (uszReceiveBuffer[usReceiveBufferSize - 1] == uszLRC)
			{
				/* 比對收到的LRC是否正確，若正確回傳ACK */
				inECR_Send_ACKorNAK(srECROb, _ACK_);

				break;
			} else
			{
				/* 比對失敗 */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "LRC error! Retry: %d", inRetry);
					inDISP_LogPrintf(szDebugMsg);

					inDISP_LogPrintf("CountLRC");
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%X", uszLRC);
					inDISP_LogPrintf(szDebugMsg);

					inDISP_LogPrintf("DataSendLRC");
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(&szDebugMsg[0], "%X", uszReceiveBuffer[usReceiveBufferSize - 1]);
					inDISP_LogPrintf(szDebugMsg);
				}

				/* 若錯誤回傳NAK */
				inECR_Send_ACKorNAK(srECROb, _NAK_);
				/* retry次數+ 1 */
				inRetry++;

				/* 初始化資料 */
				memset(uszReceiveBuffer, 0x00, sizeof (uszReceiveBuffer));
				usReceiveBufferSize = 0;
				uszSTX = VS_FALSE;
				uszETX = VS_FALSE;

				continue;
			}

		}/* 算LRC END */

	}/* Receive END */

	/* 去除STX、ETX、LRC，把資料放到szrRealReceBuffer */
	memcpy(szDataBuffer, &uszReceiveBuffer[1], usReceiveBufferSize - 3);

	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Receive_ISODeBug(szDataBuffer, usReceiveBufferSize - 3, usReceiveBufferSize - 3);

	return (VS_SUCCESS);
}

/*
Function        :inECR_Send
Date&Time       :2017/11/15 下午 5:21
Describe        :傳送要給收銀機的資料
 */
int inECR_Send(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize)
{
	int i;
	int inRetVal;
	int inRetry = 0;
	int inRecMaxRetry = _ECR_RETRYTIMES_; /* 最大重試次數 */
	int inSendLen = 0;
	char szDebugMsg[100 + 1] = {0};
	unsigned char uszSendBuf[_ECR_BUFF_SIZE_]; /* 包含STX、ETX、LRC的電文 */
	unsigned char uszLRC = 0;


	/* Send之前清Buffer，避免收到錯的回應 */
	inECR_FlushRxBuffer();

	/* 在要傳送Buffer裡放STX */
	uszSendBuf[inSendLen] = _STX_;
	inSendLen++;

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
	inSendLen++;

	/* 運算LRC(STX Not include) */
	for (i = 1; i < (inSendLen); i++)
	{
		uszLRC ^= uszSendBuf[i];
	}

	/* 在要傳送Buffer裡放LRC */
	uszSendBuf[inSendLen] = uszLRC;
	inSendLen++;

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
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "ECR Send Timeout");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
				}
				return (VS_TIMEOUT);
			}

			if (inECR_Send_Check() == VS_SUCCESS)
			{
				break;
			} else
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "ECR Send Check Not OK");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
				}
			}
		};

		/* 經由port傳送資料 */
		inRetVal = inECR_Data_Send(uszSendBuf, (unsigned short) inSendLen);

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		} else
		{

			/*---------------------------------- 印Debug 開始---------------------------------------------------------------*/
			/* 列印紙本電文和顯示電文訊息 */
			inECR_Print_Send_ISODeBug(szDataBuffer, inSendLen, inDataSize);
			/*---------------------------------- 印Debug 結束---------------------------------------------------------------*/
			/* 傳送Retry */
			if (inRetry < inRecMaxRetry)
			{
				/* 接收ACK OR NAK */
				inRetVal = inECR_Receive_ACKandNAK(srECROb);

				/* 超過一秒沒收到回應 */
				if (inRetVal == VS_TIMEOUT)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Not Receive Response, Retry");
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "Not Receive Response, Retry");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
					}

					inRetry++;
					continue;
				}					/* 收到NAK */
				else if (inRetVal == _NAK_)
				{
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("Receive NAK, Retry");
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "Receive NAK, Retry");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
					}

					inRetry++;
					continue;
				}					/* 收到ACK */
				else
				{
					/* 成功 */
					if (ginDebug == VS_TRUE)
					{
						inDISP_LogPrintf("ECR ACK OK!");
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "ECR ACK OK!");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
					}

					return (VS_SUCCESS);
				}
			}				/* 超過最大重試次數，仍要完成交易，收銀機提示補登畫面 */
				/* MPAS要沒收到ACK要印簽單 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("Exceed max retry times!");
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Exceed max retry times!");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
				}

				return (VS_ERROR);
			}

		}/*  */

	}/* while(1) */

}

/*
Function        :   inECR_Customer_Flow
Date&Time   :   2018/7/25 下午 4:26
Describe        :
 *  [國泰車麻吉] 啟用新增二段式處理流程 2022/1/28 下午 3:58 [SAM]  /
 *  20230425 [好市多加油站] Miyano 
 * 基本上只有第一段發過卡，才會進這支做預先授權
 */
int inECR_Customer_Flow(TRANSACTION_OBJECT* pobTran)
{
        int	inRetVal;
        char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
        char    szTmp[20+1]; 
        unsigned short usOneSize = 1;
        
        ECR_TABLE       srECROb2_local;     /* 收P01用 */
        ECR_TABLE       *srECROb2;
        ECR_TABLE       *srECROb;
        ECR_GASSTATION_GLOBAL_TABLE *gsrGastb;

        gsrGastb = &gsrECR_GASStation_tb;
        srECROb = &gsrECROb;
        srECROb2 = &srECROb2_local;
        
        inDISP_LogPrintfWithFlag("inECR_Customer_Flow Start");

        memset((char *)&srECROb->srTransDataGas, 0x00, sizeof(srECROb->srTransDataGas));
        
        /* 防呆 */
        srECROb->srTransDataGas.inTransType = NULL;
        srECROb->srTransDataGas.inFlag_P01 = 0;
        memset(gsrGastb->szCommand, 0x00, sizeof(gsrGastb->szCommand));
        memset((char *)srECROb2, 0x00, sizeof(srECROb2));
        
        /* 如果是不需要收第二段就Return */
        if(gsrGastb->inFlag_NextState == 0)
        {
                return VS_SUCCESS;
        }
        gsrGastb->inFlag_NextState = 0;
        memset(szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料，第二段只做預先授權------------------------------------------ */
        /* 設定Timeout */
        inDISP_Timer_Start(_TIMER_NEXSYS_4_, 60);

        while(1)
        {
                if(inTimerGet(_TIMER_NEXSYS_4_) == VS_SUCCESS)
                {
                        return VS_TIMEOUT;
                }
                
                if(inRS232_Data_Receive_Check(srECROb->srSetting.uszComPort, &usOneSize) == VS_SUCCESS)
                {
                        inRetVal = inECR_Costco_GasStation_Receive(pobTran, srECROb, szDataBuffer);

                        if (inRetVal != VS_SUCCESS)
                        {
                                return (inRetVal);
                        }

                        /* 分析資料 */
                        inECR_Costco_GasStation_Unpack(pobTran, srECROb, szDataBuffer);
                        if(srECROb->srTransDataGas.szCommand == _Costco_szCommand_PreAuth_)
                        {
                                memcpy(gsrGastb->szCommand, srECROb->srTransDataGas.szCommand, strlen(srECROb->srTransDataGas.szCommand));
                                break;
                        }
                }
        }
        /* 20230424 Miyano 進入功能或流程放這裡 */
        inCostco_GasStation_Select_TransType(pobTran, srECROb);

        /* 第一段送 */
        inRetVal = inCostco_GasStation_RS232_ECR_8N1_Send_Packet(pobTran, srECROb);
        if(inRetVal != VS_SUCCESS)
        {
                inRS232_ECR_SendError(pobTran, inRetVal);
                gsrGastb->inFlag_NextState = 0;
                return (inRetVal);
        }
        
        /* ----------------20230424 執行第一段ECR發動的指令 ---------------- */        
        if(srECROb->srTransDataGas.inTransType != NULL)
        {
                inRetVal = inCostco_GasStation_Run_TransType(pobTran, srECROb);         
                /* 處理ECR發動的指令 */
                inCostco_GasStation_Handle_CommandResult(pobTran, srECROb, inRetVal);
        }
        
        /* 收P01 並回應ECR，如果已經發P01就跳過 */
        if(memcmp(srECROb->srTransDataGas.szCommand, "P01", 3))
        {
                while(1)
                {
                        inRetVal = inCostco_GasStation_ECR_2nd_Receive_P01(pobTran, srECROb2);
                        if(inRetVal == VS_TIMEOUT)
                        {
                                inDISP_LogPrintf("Wait P01 Timeout");
                                gsrGastb->inFlag_NextState = 0;
                                return VS_TIMEOUT;
                        }
                        else if(inRetVal == VS_ERROR)
                        {
                                memset(szTmp, 0x00, sizeof(szTmp));
                                memcpy(szTmp, srECROb->srTransDataGas.szCommand, strlen(srECROb->srTransDataGas.szCommand));
                                memcpy(srECROb->srTransDataGas.szCommand, "Busy", 4);
                                inCostco_GasStation_RS232_ECR_8N1_Send_Packet2(pobTran, srECROb);
                                memset(srECROb->srTransDataGas.szCommand, 0x00, sizeof(srECROb->srTransDataGas.szCommand));
                                memcpy(srECROb->srTransDataGas.szCommand, szTmp, strlen(szTmp));
                        }
                        else if(inRetVal == VS_SUCCESS)
                        {
                                break;
                        }
                }
                inCostco_GasStation_RS232_ECR_8N1_Send_Packet2(pobTran, srECROb);
        }
        
        inDISP_LogPrintfWithFlag("inECR_Customer_Flow End");
        return (VS_SUCCESS);
}

/*
Function        :   inECR_Customer_Flow2
Date&Time       :   2018/7/25 下午 4:26
Describe        :
 *      202300502 [好市多加油站] Miyano 
 *      基本上只有第二段預先授權成功，才會進這支做授權完成
 */
int inECR_Customer_Flow2(TRANSACTION_OBJECT* pobTran)
{
        int	inRetVal;
        char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
        char    szTmp[20+1]; 
        unsigned short usOneSize = 1;
        
        ECR_TABLE       srECROb2_local;     /* 收P01用 */
        ECR_TABLE       *srECROb2;
        ECR_TABLE       *srECROb;
        ECR_GASSTATION_GLOBAL_TABLE *gsrGastb;

        gsrGastb = &gsrECR_GASStation_tb;
        srECROb = &gsrECROb;
        srECROb2 = &srECROb2_local;
        
        inDISP_LogPrintfWithFlag("inECR_Customer_Flow2 Start");

        memset((char *)&srECROb->srTransDataGas, 0x00, sizeof(srECROb->srTransDataGas));
        
        /* 防呆 */
        srECROb->srTransDataGas.inTransType = NULL;
        srECROb->srTransDataGas.inFlag_P01 = 0;
        memset(gsrGastb->szCommand, 0x00, sizeof(gsrGastb->szCommand));
        memset((char *)srECROb2, 0x00, sizeof(srECROb2));
        
        /* 如果是不需要收第二段就Return */
        if(gsrGastb->inFlag_NextState == 0)
        {
                return VS_SUCCESS;
        }
        gsrGastb->inFlag_NextState = 0;
        memset(szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料，第二段只做預先授權------------------------------------------ */
        /* 設定Timeout */
        inDISP_Timer_Start(_TIMER_NEXSYS_4_, 60);

        while(1)
        {
                if(inTimerGet(_TIMER_NEXSYS_4_) == VS_SUCCESS)
                {
                        return VS_TIMEOUT;
                }
                
                if(inRS232_Data_Receive_Check(srECROb->srSetting.uszComPort, &usOneSize) == VS_SUCCESS)
                {
                        inRetVal = inECR_Costco_GasStation_Receive(pobTran, srECROb, szDataBuffer);

                        if (inRetVal != VS_SUCCESS)
                        {
                                return (inRetVal);
                        }

                        /* 分析資料 */
                        inECR_Costco_GasStation_Unpack(pobTran, srECROb, szDataBuffer);
                        if(srECROb->srTransDataGas.szCommand == _Costco_szCommand_AuthComplete_)
                        {
                                memcpy(gsrGastb->szCommand, srECROb->srTransDataGas.szCommand, strlen(srECROb->srTransDataGas.szCommand));
                                break;
                        }
                }
        }
        /* 20230424 Miyano 進入功能或流程放這裡 */
        inCostco_GasStation_Select_TransType(pobTran, srECROb);

        /* 第一段送 */
        inRetVal = inCostco_GasStation_RS232_ECR_8N1_Send_Packet(pobTran, srECROb);
        if(inRetVal != VS_SUCCESS)
        {
                inRS232_ECR_SendError(pobTran, inRetVal);
                return (inRetVal);
        }
        
        /* ----------------20230424 執行第一段ECR發動的指令 ---------------- */        
        if(srECROb->srTransDataGas.inTransType != NULL)
        {
                inRetVal = inCostco_GasStation_Run_TransType(pobTran, srECROb);         
                /* 處理ECR發動的指令 */
                inCostco_GasStation_Handle_CommandResult(pobTran, srECROb, inRetVal);
        }
        
        /* 收P01 並回應ECR，如果已經發P01就跳過 */
        if(memcmp(srECROb->srTransDataGas.szCommand, "P01", 3))
        {
                while(1)
                {
                        inRetVal = inCostco_GasStation_ECR_2nd_Receive_P01(pobTran, srECROb2);
                        if(inRetVal == VS_TIMEOUT)
                        {
                                inDISP_LogPrintf("Wait P01 Timeout");
                                gsrGastb->inFlag_NextState = 0;
                                return VS_TIMEOUT;
                        }
                        else if(inRetVal == VS_ERROR)
                        {
                                memset(szTmp, 0x00, sizeof(szTmp));
                                memcpy(szTmp, srECROb->srTransDataGas.szCommand, strlen(srECROb->srTransDataGas.szCommand));
                                memcpy(srECROb->srTransDataGas.szCommand, "Busy", 4);
                                inCostco_GasStation_RS232_ECR_8N1_Send_Packet2(pobTran, srECROb);
                                memset(srECROb->srTransDataGas.szCommand, 0x00, sizeof(srECROb->srTransDataGas.szCommand));
                                memcpy(srECROb->srTransDataGas.szCommand, szTmp, strlen(szTmp));
                        }
                        else if(inRetVal == VS_SUCCESS)
                        {
                                break;
                        }
                }
                inCostco_GasStation_RS232_ECR_8N1_Send_Packet2(pobTran, srECROb);
        }
        
        inDISP_LogPrintfWithFlag("inECR_Customer_Flow2 End");
        return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_Standard_Unpack
Date&Time       :2017/11/15 下午 2:44
Describe        :分析收銀機傳來的資料
 */
int inECR_8N1_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int i = 0;
	int inRetVal = VS_SUCCESS;
	int inTransType = 0; /* 電文中的交易別字串轉為數字儲存 */
	char szTemplate[100 + 1];
	char szDebugMsg[100 + 1];
	char szHostLabel[8 + 1];
	char szHostEnable[2 + 1];
	char szBatchNum[6 + 1];
	char szCTLSEnable[2 + 1];

	/* ECR Indicator :新 ECR連線 Indicator"I"(規格新增欄位 )。 */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[0], 1);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "ECR Indicator", szTemplate);
				inDISP_LogPrintf(szDebugMsg);
			}
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "ECR Indicator", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			}

			/* ECR Indicator必為'I'，否則error */
			if (memcmp(&szTemplate[0], "I", 1) != 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "ECR Indicator", "Not \"I\" Error");
					inDISP_LogPrintf(szDebugMsg);
				}
				return (VS_ERROR);
			}
			break;
	}

	/* Trans Type (交易別) */
	switch (inTransType)
	{
		default:
			memset(srECROb->srTransData.szTransType, 0x00, sizeof (srECROb->srTransData.szTransType));
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[8], 2);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			} else
			{
				memcpy(srECROb->srTransData.szTransType, szTemplate, 2);
			}

			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);

			/* 要連動結帳 */
			if (inTransType == _ECR_8N1_SETTLEMENT_NO_)
			{
				pobTran->uszAutoSettleBit = VS_TRUE;
			}

			break;
	}

	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
	}

	/* 啟動卡號查詢不受Trans Indicator影響 */
	if (inTransType == _ECR_8N1_START_CARD_NO_INQUIRY_NO_)
	{
		/* 卡號查詢 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		memcpy(&srECROb->srTransData.szStartTransType[0], &szDataBuffer[220], 2);
		memcpy(&szTemplate[0], &szDataBuffer[220], 2);
		inTransType = atoi(szTemplate);
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_:
			case _ECR_8N1_REFUND_NO_:
			case _ECR_8N1_PREAUTH_NO_:
			case _ECR_8N1_PREAUTH_COMPLETE_NO_:
				if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
				{

				} else
				{
					pobTran->uszCardInquiryFirstBit = VS_TRUE;
				}
				/* 有值影響到後面判斷  確保都是空格 */
				memcpy(&srECROb->srTransData.szCUPIndicator[0], " ", 1);
				memcpy(&srECROb->srTransData.szTransIndicator[0], " ", 1);
				break;
			case _ECR_8N1_OFFLINE_NO_:
			case _ECR_8N1_INSTALLMENT_NO_:
			case _ECR_8N1_REDEEM_NO_:
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			case _ECR_8N1_REDEEM_REFUND_NO_:
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
			case _ECR_8N1_REDEEM_ADJUST_NO_:
				if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
				{

				} else
				{
					pobTran->uszCardInquiryFirstBit = VS_TRUE;
				}
				memcpy(&srECROb->srTransData.szCUPIndicator[0], " ", 1);
				memcpy(&srECROb->srTransData.szTransIndicator[0], " ", 1);
				break;
			default:
				return (VS_ERROR);
		}
	} else
	{
		/* Trans Type Indicator :支援交易別 :一般交易、補登退貨 ，其餘 交易別，端末機不參考此欄位 。*/
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_: /* 一般交易 */
			case _ECR_8N1_REFUND_NO_: /* 退貨交易 */
			case _ECR_8N1_OFFLINE_NO_: /* 交易補登 */
				memset(srECROb->srTransData.szTransIndicator, 0x00, sizeof (srECROb->srTransData.szTransIndicator));
				memcpy(srECROb->srTransData.szTransIndicator, &szDataBuffer[7], 1);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type Indicator", srECROb->srTransData.szTransIndicator);
					inDISP_LogPrintf(szDebugMsg);
				}
				/* Trans Type Indicator是'S'，要進選擇畫面 */
				if (!memcmp(&srECROb->srTransData.szTransIndicator[0], "S", 1))
				{
					memset(srECROb->srTransData.szTransIndicator, 0x00, sizeof (srECROb->srTransData.szTransIndicator));
					inRetVal = inRS232_ECR_SelectTransType(srECROb);
					/* 交易類別轉成數字 */
					inTransType = atoi(srECROb->srTransData.szTransType);

					/* Select失敗 */
					if (inRetVal != VS_SUCCESS)
					{
						return (inRetVal);
					}
				}
				break;
			default:
				break;
		}

		/* 要判斷是否空值，有可能Trans Type Indicator就會順便塞值 */
		if (memcmp(&srECROb->srTransData.szCUPIndicator, "\x00", 1) == 0)
		{
			/* CUP/Smart pay Indicator:銀聯卡 / Smart Pay交易使用
			 * Indicator = 'C'，表示為CUP交易
			 * Indicator = 'N'，表示為一般信用卡交易
			 * Indicator = 'S'，表示為SmartPay交易，(SmartPay的Sale Reversal和Refund，收銀機一定要送'S'。SmartPay的Sale送'N'，因為Sale不需要按Hotkey)
			 */
			switch (inTransType)
			{
				case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* 啟動卡號查詢 */
				case _ECR_8N1_SALE_NO_: /* 一般交易 */
				case _ECR_8N1_REFUND_NO_: /* 退貨交易 */
				case _ECR_8N1_VOID_NO_: /* 取消 */
				case _ECR_8N1_PREAUTH_NO_: /* 預先授權 */
				case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 */
				case _ECR_8N1_ESVC_TOP_UP_NO_: /* 電子票證加值 */
				case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_: /* 電子票證餘額查詢 */
				case _ECR_8N1_ESVC_VOID_TOP_UP_NO_: /* 電子票證加值取消 */
				case _ECR_8N1_HG_REWARD_SALE_NO_: /* HG紅利積點 */
				case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_: /* HG點數抵扣 */
				case _ECR_8N1_HG_POINT_CERTAIN_NO_: /* HG加價購 */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(szTemplate, &szDataBuffer[10], 1);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", szTemplate);
						inDISP_LogPrintf(szDebugMsg);
					}

					/* 欄位為M 卻不存在，回傳錯誤 */
					if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "Not Exist Error");
							inDISP_LogPrintf(szDebugMsg);
						}

						return (VS_ERROR);
					} else
					{
						memcpy(srECROb->srTransData.szCUPIndicator, szTemplate, 1);
					}

					break;
				case _ECR_8N1_PREAUTH_CANCEL_NO_: /* 預先授權取消 */
				case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_: /* 預先授權完成取消 */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(szTemplate, &szDataBuffer[10], 1);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "  CUPIndicator :%s  ", szTemplate);
						inDISP_LogPrintf(szDebugMsg);
					}

					/* 欄位為M 卻不存在，回傳錯誤 */
					if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "Not Exist Error");
							inDISP_LogPrintf(szDebugMsg);
						}

						return (VS_ERROR);
					} else
					{
						memcpy(srECROb->srTransData.szCUPIndicator, szTemplate, 1);
					}

					/* 預先授權取消及完成僅支援銀聯卡交易 ，故此二Request之 CUP Indicator欄位僅可放 ”C”*/
					if (memcmp(&srECROb->srTransData.szCUPIndicator[0], "C", 1) != 0)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "CUP Not\"C\" Error");
							inDISP_LogPrintf(szDebugMsg);
						}

						return (VS_ERROR);
					}
					break;
				default:
					break;
			}
		}

		/* 如果是'0'代表Hotkey選擇 */
		if (memcmp(&srECROb->srTransData.szCUPIndicator[0], "0", 1) == 0)
		{
			inRetVal = inRS232_ECR_SelectOtherCardType(pobTran);

			/* Select失敗 */
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
		}

		/* 如果是'C'代表是銀聯卡交易 */
		if (memcmp(&srECROb->srTransData.szCUPIndicator[0], "C", 1) == 0)
		{
			pobTran->srBRec.uszCUPTransBit = VS_TRUE;
		}

		/* 如果是'S'代表是SmartPay交易 */
		if (memcmp(&srECROb->srTransData.szCUPIndicator[0], "S", 1) == 0)
		{
			pobTran->srBRec.uszFiscTransBit = VS_TRUE;
			/* SmartPay不用簽名 */
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
		}

		/* 如果是'E'代表是電票交易 */
		if (memcmp(&srECROb->srTransData.szCUPIndicator[0], "E", 1) == 0)
		{
			pobTran->srTRec.uszESVCTransBit = VS_TRUE;
		}
	}

	/* 銀行別/付款別 HOST ID*/
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_: /* 取消交易 */
		case _ECR_8N1_TIP_NO_: /* 小費交易 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_: /* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_: /* 預先授權完成取消 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_: /* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_: /* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_: /* 總額查詢 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[11], 2);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			}

			memset(szHostLabel, 0x00, sizeof (szHostLabel));

			if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_));
			} else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_DCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
			}				/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_LOYALTY_REDEEM_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_));
			} else
			{
				return (VS_ERROR);
			}

			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					/* 如果return VS_ERROR 代表table沒有該HOST */
					inDISP_LogPrintfWithFlag(" ECR 8n1 Standard Unpack Load HDT[%d] *Error* Line[%d]", i, __LINE__);
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof (szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof (szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inDISP_LogPrintf(szDebugMsg);
						}

						/* 如果return VS_ERROR 代表Host沒開 */
						return (VS_ERROR);
					} else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;

						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof (szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inDISP_LogPrintf(szDebugMsg);
						}

					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;

		default:
			break;
	}

	/* EDC簽單序號 Receipt No(InvoiceNumber) */
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_: /* 取消交易 */
		case _ECR_8N1_TIP_NO_: /* 小費交易 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_: /* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_: /* 預先授權完成取消 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_: /* 重印簽單 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(&szTemplate[0], &szDataBuffer[13], 6);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			} else
			{
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;
		default:
			break;
	}

	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* 啟動卡號查詢 */
		case _ECR_8N1_SALE_NO_: /* 一般交易 */
		case _ECR_8N1_REFUND_NO_: /* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_: /* 交易補登 */
		case _ECR_8N1_PREAUTH_NO_: /* 預先授權 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 */
		case _ECR_8N1_INSTALLMENT_NO_: /* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_: /* 紅利 */
		case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
		case _ECR_8N1_ESVC_TOP_UP_NO_: /* 電子票證加值 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_: /* 電子票證加值取消 */
		case _ECR_8N1_HG_REWARD_SALE_NO_: /* HG紅利積點 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_: /* 快樂購紅利積點 + 信用卡分期付款 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_: /* 快樂購紅利積點 + 信用卡紅利扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_: /* HG點數抵扣 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_: /* HG加價購 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(&szTemplate[0], &szDataBuffer[42], 10);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			} else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;
		default:
			break;
	}

	/* 交易日期 Trans Date */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* 啟動卡號查詢 */
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_: /* 結束卡號查詢*/
		case _ECR_8N1_SALE_NO_: /* 一般交易 */
		case _ECR_8N1_REFUND_NO_: /* 退貨交易 */
		case _ECR_8N1_VOID_NO_: /* 取消交易 */
		case _ECR_8N1_OFFLINE_NO_: /* 交易補登 */
		case _ECR_8N1_TIP_NO_: /* 小費交易 */
		case _ECR_8N1_PREAUTH_NO_: /* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_: /* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_: /* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_: /* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_: /* 紅利 */
		case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
		case _ECR_8N1_SETTLEMENT_NO_: /* 結帳交易 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_: /* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_: /* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_: /* 總額查詢 */
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_: /* 明細列印 */
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_: /* 總額列印 */
		case _ECR_8N1_ESVC_TOP_UP_NO_: /* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_: /* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_: /* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(&szTemplate[0], &szDataBuffer[54], 6);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Date", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				//				return (VS_ERROR);
			} else
			{
				sprintf(pobTran->srBRec.szDate, "20%s", szTemplate);
				sprintf(pobTran->srBRec.szOrgDate, "20%s", szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易日期", pobTran->srBRec.szDate);
				inDISP_LogPrintf(szDebugMsg);
			}
			break;
		default:
			break;
	}

	/* 交易時間 Trans Time */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* 啟動卡號查詢 */
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_: /* 結束卡號查詢*/
		case _ECR_8N1_SALE_NO_: /* 一般交易 */
		case _ECR_8N1_REFUND_NO_: /* 退貨交易 */
		case _ECR_8N1_VOID_NO_: /* 取消交易 */
		case _ECR_8N1_OFFLINE_NO_: /* 交易補登 */
		case _ECR_8N1_TIP_NO_: /* 小費交易 */
		case _ECR_8N1_PREAUTH_NO_: /* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_: /* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_: /* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_: /* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_: /* 紅利 */
		case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
		case _ECR_8N1_SETTLEMENT_NO_: /* 結帳交易 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_: /* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_: /* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_: /* 總額查詢 */
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_: /* 明細列印 */
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_: /* 總額列印 */
		case _ECR_8N1_ESVC_TOP_UP_NO_: /* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_: /* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_: /* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[60], 6);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Time", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				//				return (VS_ERROR);
			} else
			{
				sprintf(pobTran->srBRec.szTime, "%s", szTemplate);
				sprintf(pobTran->srBRec.szOrgTime, "%s", szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易時間", pobTran->srBRec.szTime);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;

		default:
			break;
	}

	/* 授權碼(Auth Code) ECR電文中給9個字元 但ATS電文只需要6個，所以只抓6個 */
	switch (inTransType)
	{
		case _ECR_8N1_REFUND_NO_: /* 退貨 */
		case _ECR_8N1_OFFLINE_NO_: /* 交易補登 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
		case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof (pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[66], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			break;
		case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 */
			/* 處理授權碼(must have) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof (pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[66], 6);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Code", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			} else
			{
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}

			break;

		default:
			break;
	}

	/* 其他金額(Exp Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_TIP_NO_: /* 小費交易 */
			/* 小費金額*/
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memset(szTemplate, 0x00, sizeof (szTemplate));
			inGetTRTFileName(szTemplate);
			if (!memcmp(szTemplate, "DCCTRT", 6) && !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
			{
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[103], 12);
			} else
			{
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[103], 10);
			}

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Exp Amount", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			}

			pobTran->srBRec.lnTipTxnAmount = atol(szTemplate);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tip: %ld", pobTran->srBRec.lnTipTxnAmount);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;

		case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 預先授權完成交易之原預先授權金額 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[103], 10);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Exp Amount", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			}

			pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
			break;

		default:
			break;
	}

	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* 啟動卡號查詢 */
		case _ECR_8N1_SALE_NO_: /* 一般交易 */
		case _ECR_8N1_REFUND_NO_: /* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_: /* 交易補登 */
		case _ECR_8N1_PREAUTH_NO_: /* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_: /* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_: /* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_: /* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_: /* 紅利 */
		case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
		case _ECR_8N1_HG_REWARD_SALE_NO_: /* HG紅利積點 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_: /* 快樂購紅利積點 + 信用卡分期付款 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_: /* 快樂購紅利積點 + 信用卡紅利扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_: /* 快樂購點數扣抵 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_: /* 快樂購加價購 */
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_: /* 快樂購點數兌換 */
		case _ECR_8N1_HG_REDEEM_REFUND_NO_: /* 快樂購扣抵退貨 */
		case _ECR_8N1_HG_REWARD_REFUND_NO_: /* 快樂購回饋退貨 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[115], 18);

			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_: /* 電子票證加值 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_: /* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[115], 18);

			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srTRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srTRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srTRec.szStoreID);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;
		default:
			break;
	}

	if (inTransType == _ECR_8N1_AWARD_REDEEM_NO_)
	{

	} else if (inTransType == _ECR_8N1_VOID_AWARD_REDEEM_NO_)
	{

	} else
	{
		/* 實際支付金額RDM Paid Amt (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
			case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_: /* 快樂購紅利積點 + 信用卡紅利扣抵 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[134], 10); /* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "RDM Paid Amt", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 信用卡紅利扣抵點數 RDM Point */
		switch (inTransType)
		{
			case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
			case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_: /* 快樂購紅利積點 + 信用卡紅利扣抵 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[146], 8);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "RDM Point", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 分期期數 Installment Period */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_NO_: /* 分期 */
			case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_: /* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[174], 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Installment Period", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 首期金額 Down Payment Amount (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_: /* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[176], 10); /* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Down Payment Amount", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 每期金額 Installment Payment Amount (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_: /* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[188], 10); /* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Installment Payment Amount", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 分期手續費 Formallity Fee (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[200], 10); /* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}
				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

				if (pobTran->srBRec.lnInstallmentFormalityFee != 0)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_;

					return (VS_ERROR);
				}

				break;
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_: /* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[200], 10); /* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 啟動卡號查詢 Start Trans Type */
		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* 卡號查詢 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[220], 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Start Trans Type", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				memcpy(srECROb->srTransData.szStartTransType, szTemplate, 2);

				break;
			default:
				break;
		}

		/* 只有金融卡才要收的欄位 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/* 金融卡原交易日期 SP Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_: /* 退費 */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(szTemplate, &szDataBuffer[231], 8);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "SP Origin Date", szTemplate);
						inDISP_LogPrintf(szDebugMsg);
					}

					memcpy(pobTran->srBRec.szFiscRefundDate, szTemplate, 8);
					break;
				default:
					break;
			}

			/* 金融卡調單編號 SP RRN */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_: /* 退費 */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(szTemplate, &szDataBuffer[239], 12);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "SP RRN", szTemplate);
						inDISP_LogPrintf(szDebugMsg);
					}

					memcpy(pobTran->srBRec.szFiscRRN, szTemplate, 12);

					break;
				default:
					break;
			}

		}			/* 只有電票要收的欄位 */
		else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* ATS電票交易序號 RF NUMBER 左靠右補空白 */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_: /* 電票退貨 */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(szTemplate, &szDataBuffer[239], 12);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "RF NUMBER", szTemplate);
						inDISP_LogPrintf(szDebugMsg);
					}

					memcpy(pobTran->srTRec.szTicketRefundCode, szTemplate, 12);

					break;
				default:
					break;
			}
		}

	}
	/* 因為要修改掉NCCC的規則,所以拿掉 20190328 [SAM] */
#if 0	
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* 有開CUP且MACEnable有開但安全認證沒過，不能執行CUP交易 */
		if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
		{
			if (inNCCC_Func_CUP_PowerOn_LogOn(pobTran) != VS_SUCCESS)
			{
				/* 安全認證失敗 */
				return (VS_ERROR);
			}
		}
	}
#endif 	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_8N1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
				pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;

				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜銀聯一般交易＞ */

				pobTran->inFunctionID = _CUP_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = _CUP_SALE_;
				pobTran->srBRec.inOrgCode = _CUP_SALE_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
				} else
				{
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
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

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}

			break;
		case _ECR_8N1_VOID_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜沖正交易＞ */

				pobTran->inFunctionID = _FISC_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_FISC_VOID_;

				pobTran->inTransactionCode = _FISC_VOID_;
				pobTran->srBRec.inCode = _FISC_VOID_;
				pobTran->srBRec.inOrgCode = _FISC_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = _CUP_VOID_;
				pobTran->srBRec.inOrgCode = _CUP_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_8N1_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _CUP_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
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

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜退貨交易＞ */

				pobTran->inFunctionID = _TICKET_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_REFUND_;
				} else
				{
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_REFUND_;
				pobTran->srTRec.inCode = _TICKET_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
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

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_8N1_OFFLINE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜交易補登＞ */

			pobTran->inFunctionID = _SALE_OFFLINE_;
			pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

			pobTran->inTransactionCode = _SALE_OFFLINE_;
			pobTran->srBRec.inCode = _SALE_OFFLINE_;
			pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_PREAUTH_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權＞ */

				pobTran->inFunctionID = _PRE_AUTH_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inTransactionCode = _PRE_AUTH_;
				pobTran->srBRec.inCode = _PRE_AUTH_;
				pobTran->srBRec.inOrgCode = _PRE_AUTH_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權＞ */

				pobTran->inFunctionID = _PRE_AUTH_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inTransactionCode = _PRE_AUTH_;
				pobTran->srBRec.inCode = _PRE_AUTH_;
				pobTran->srBRec.inOrgCode = _PRE_AUTH_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}

			break;

		case _ECR_8N1_PREAUTH_CANCEL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_AUTH_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權取消＞ */

			pobTran->inFunctionID = _CUP_PRE_AUTH_VOID_;
			pobTran->inRunOperationID = _OPERATION_VOID_;
			pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_VOID_;

			pobTran->inTransactionCode = _CUP_PRE_AUTH_VOID_;
			pobTran->srBRec.inCode = _CUP_PRE_AUTH_VOID_;
			pobTran->srBRec.inOrgCode = _CUP_PRE_AUTH_VOID_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_PREAUTH_COMPLETE_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權完成＞ */

				pobTran->inFunctionID = _CUP_PRE_COMP_;
				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_;
				}
				pobTran->inRunTRTID = _TRT_PRE_COMP_;

				pobTran->inTransactionCode = _CUP_PRE_COMP_;
				pobTran->srBRec.inCode = _CUP_PRE_COMP_;
				pobTran->srBRec.inOrgCode = _CUP_PRE_COMP_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權完成＞ */

				pobTran->inFunctionID = _PRE_COMP_;
				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_;
				}
				pobTran->inRunTRTID = _TRT_PRE_COMP_;

				pobTran->inTransactionCode = _PRE_COMP_;
				pobTran->srBRec.inCode = _PRE_COMP_;
				pobTran->srBRec.inOrgCode = _CUP_PRE_COMP_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_COMP_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權完成取消＞ */

			pobTran->inFunctionID = _CUP_PRE_COMP_VOID_;
			pobTran->inRunOperationID = _OPERATION_VOID_;
			pobTran->inRunTRTID = _TRT_CUP_PRE_COMP_VOID_;

			pobTran->inTransactionCode = _CUP_PRE_COMP_VOID_;
			pobTran->srBRec.inCode = _CUP_PRE_COMP_VOID_;
			pobTran->srBRec.inOrgCode = _CUP_PRE_COMP_VOID_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_INSTALLMENT_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 保留給銀聯分期使用 */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 保留給銀聯分期退貨使用 */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期調帳＞ */

			pobTran->inFunctionID = _INST_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_INST_ADJUST_;
			pobTran->inRunTRTID = _TRT_INST_ADJUST_;

			pobTran->inTransactionCode = _INST_ADJUST_;
			pobTran->srBRec.inCode = _INST_ADJUST_;
			pobTran->srBRec.inOrgCode = _INST_ADJUST_;

			pobTran->srBRec.uszInstallmentBit = VS_TRUE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_REDEEM_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 保留給銀聯紅利扣抵使用 */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_8N1_REDEEM_REFUND_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 保留給銀聯紅利退貨使用 */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_CUP_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_8N1_REDEEM_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利調帳＞ */

			pobTran->inFunctionID = _REDEEM_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_REDEEM_ADJUST_;
			pobTran->inRunTRTID = _TRT_REDEEM_ADJUST_;

			pobTran->inTransactionCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inOrgCode = _REDEEM_ADJUST_;

			pobTran->srBRec.uszRedeemBit = VS_TRUE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_TIP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TIP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜小費交易＞ */

			pobTran->inFunctionID = _TIP_;
			pobTran->inRunOperationID = _OPERATION_TIP_;
			pobTran->inRunTRTID = _TRT_TIP_;

			pobTran->inTransactionCode = _TIP_;
			pobTran->srBRec.inCode = _TIP_;
			pobTran->srBRec.inOrgCode = _TIP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_AWARD_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜優惠兌換＞ */

			pobTran->inFunctionID = _LOYALTY_REDEEM_;
			pobTran->inRunOperationID = _OPERATION_LOYALTY_REDEEM_CTLS_;
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;

			pobTran->inTransactionCode = _LOYALTY_REDEEM_;
			pobTran->srBRec.inCode = _LOYALTY_REDEEM_;
			pobTran->srBRec.inOrgCode = _LOYALTY_REDEEM_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_VOID_AWARD_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_VOID_LOYALTY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜兌換取消＞ */

			pobTran->inFunctionID = _VOID_LOYALTY_REDEEM_;
			pobTran->inRunOperationID = _OPERATION_VOID_LOYALTY_REDEEM_;
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;

			pobTran->inTransactionCode = _VOID_LOYALTY_REDEEM_;
			pobTran->srBRec.inCode = _VOID_LOYALTY_REDEEM_;
			pobTran->srBRec.inOrgCode = _VOID_LOYALTY_REDEEM_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;


			/* 快樂購紅利積點 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡紅利積點＞ */

			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_CREDIT_CTLS_;

			/* 這邊先設成sale，若之後支付方式選銀聯卡，會在那邊改成CUP_SALE */
			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購紅利積點─分期付款 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡分期付款＞ */

			pobTran->inFunctionID = _INST_SALE_;
			pobTran->inRunOperationID = _OPERATION_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_INSTALLMENT_CTLS_;

			pobTran->inTransactionCode = _INST_SALE_;
			pobTran->srBRec.inCode = _INST_SALE_;
			pobTran->srBRec.inOrgCode = _INST_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購紅利積點─紅利扣抵 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡紅利扣抵＞ */

			pobTran->inFunctionID = _REDEEM_SALE_;
			pobTran->inRunOperationID = _OPERATION_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REDEMPTION_CTLS_;

			pobTran->inTransactionCode = _REDEEM_SALE_;
			pobTran->srBRec.inCode = _REDEEM_SALE_;
			pobTran->srBRec.inOrgCode = _REDEEM_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszRedeemBit = VS_TRUE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購點數扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡點數扣抵＞ */

			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_ONLINE_REDEEM_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_ONLINE_REDEEM_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購加價購 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡加價購＞ */

			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_POINT_CERTAIN_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_POINT_CERTAIN_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購點數兌換 */
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡點數兌換＞ */

			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_FULL_REDEMPTION_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_FULL_REDEMPTION_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購扣抵退貨 */
		case _ECR_8N1_HG_REDEEM_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡扣抵退貨＞ */

			pobTran->inFunctionID = _HG_REDEEM_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REDEEM_REFUND_;

			pobTran->inTransactionCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REDEEM_REFUND_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購回饋退貨 */
		case _ECR_8N1_HG_REWARD_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡回饋退貨＞ */

			pobTran->inFunctionID = _HG_REWARD_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REFUND_;

			pobTran->inTransactionCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_REFUND_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購點數查詢 */
		case _ECR_8N1_HG_POINT_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡點數查詢＞ */

			pobTran->inFunctionID = _HG_INQUIRY_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_INQUIRY_;

			pobTran->inTransactionCode = _HG_INQUIRY_;
			pobTran->srBRec.inCode = _HG_INQUIRY_;
			pobTran->srBRec.inOrgCode = _HG_INQUIRY_;
			pobTran->srBRec.lnHGTransactionType = _HG_INQUIRY_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;

			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_REPRINT_RECEIPT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜重印帳單＞ */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REPRINT_;
			pobTran->inRunTRTID = FALSE;

			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 <交易查詢> */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_DETAIL_;
			pobTran->inRunTRTID = FALSE;

			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜總額查詢＞ */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_TOTAL_;
			pobTran->inRunTRTID = FALSE;

			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 明細列印 */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_DETAIL_REPORT_;
			pobTran->inRunTRTID = FALSE;

			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 總額列印 */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_TOTAL_REPORT_;
			pobTran->inRunTRTID = FALSE;

			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_EDC_REBOOT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜重新開機＞ */
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REBOOT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜重新開機＞ */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunTRTID = FALSE;

			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;

			/* 直接用function 不跑OPT */
			inFunc_Reboot();

			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜加值交易＞ */

			pobTran->inFunctionID = _TICKET_TOP_UP_;

			memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
			inGetContactlessEnable(szCTLSEnable);
			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_TOP_UP_;
			} else
			{
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_TOP_UP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜餘額查詢＞ */

			pobTran->inFunctionID = _TICKET_INQUIRY_;

			memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
			inGetContactlessEnable(szCTLSEnable);
			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_INQUIRY_;
			} else
			{
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_INQUIRY_;
			pobTran->srTRec.inCode = _TICKET_INQUIRY_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜加值取消＞ */

			pobTran->inFunctionID = _TICKET_VOID_TOP_UP_;

			memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
			inGetContactlessEnable(szCTLSEnable);
			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_VOID_TOP_UP_;
			} else
			{
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_VOID_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_VOID_TOP_UP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_;
			inRetVal = VS_ERROR;
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}

	return (inRetVal);
}

/*
Function        :inRS232_ECR_8N1_Standard_Pack
Date&Time       :2016/7/6 下午 2:40
Describe        :先把要送的資料組好
 */
int inECR_8N1_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int inRetVal = VS_ERROR;
	int i = 0, inCardLen = 0;
	int inPacketSizes = 0;
	char szTemplate[100 + 1];
	char szHash[44 + 1];
	char szFESMode[2 + 1];

	/* ECR Indicator (1 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "I", 1);
	inPacketSizes++;
	/* ECR Version Date (6 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "160407", 6);
	inPacketSizes += 6;
	/* Trans Type Indicator (1 Byte) */
	inPacketSizes++;
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;
	/* CUP Indicator (1 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2))
	{
		if (!memcmp(srECROb->srTransData.szCUPIndicator, "C", 1) || pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "C", 1);
		else if (!memcmp(srECROb->srTransData.szCUPIndicator, "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "S", 1);
		else if (!memcmp(srECROb->srTransData.szCUPIndicator, "E", 1) || pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "E", 1);
		else
			memcpy(&szDataBuffer[inPacketSizes], "N", 1);
	}

	inPacketSizes++;

	/* HOST ID (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetTRTFileName(szTemplate);

		if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_NCCC_, 2);
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_HG_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_DCC_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_ESVC_, 2);
	}

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			sprintf(szTemplate, "%06ld", pobTran->srTRec.lnInvNum);
		} else
		{
			sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		}
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Card No (19 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2) ||
			pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
	{
		/* 優惠兌換先設定不回傳卡號 */
		inPacketSizes += 19;
	} else
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTRec.szUID[0], strlen(pobTran->srTRec.szUID));
		} else
		{
			/* 卡號是否遮掩 */
			inRetVal = inECR_CardNoTruncateDecision(pobTran);
			/* 要遮卡號 */
			if (inRetVal == VS_SUCCESS)
			{
				memset(szTemplate, 0x00, sizeof (szTemplate));
				strcpy(szTemplate, pobTran->srBRec.szPAN);

				/* HAPPG_GO 卡不掩飾 */
				if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
				{

				} else
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
						for (i = 2; i < (inCardLen - 4); i++)
							szTemplate[i] = '*';
					} else
					{
						for (i = 6; i < (inCardLen - 4); i++)
							szTemplate[i] = '*';
					}

				}

				memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
			} else
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
			}
		}
		inPacketSizes += 19;
	}

	/* Card Expire Date (4 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2))
	{
		inPacketSizes += 4;
	} else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		inPacketSizes += 4;
	} else
	{
		/* 有效期是否回傳 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetECRExpDateReturnEnable(szTemplate);

		/* 要回傳有效期 */
		if (!memcmp(&szTemplate, "Y", 1))
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

		inPacketSizes += 4;
	}

	/* Trans Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2))
	{
		inPacketSizes += 12;
	} else if (pobTran->srTRec.lnTxnAmount != 0)
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srTRec.lnTxnAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

		inPacketSizes += 12;
	} else if (pobTran->srBRec.lnTxnAmount != 0)
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

		inPacketSizes += 12;
	} else
	{
		inPacketSizes += 12;
	}

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2))
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* Trans Date */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			strcat(szTemplate, pobTran->srTRec.szDate);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;

			/* Trans Time */
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srTRec.szTime, 6);
			inPacketSizes += 6;
		} else
		{
			/* Trans Date */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			strcat(szTemplate, pobTran->srBRec.szDate);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
			inPacketSizes += 6;

			/* Trans Time */
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
			inPacketSizes += 6;
		}
	} else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			strcpy(szTemplate, pobTran->srTRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PAD_LEFT_FILL_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		} else
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PAD_LEFT_FILL_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
	}

	inPacketSizes += 9;

	/* Wave Card Indicator (1 Byte) */
	/* Add by hanlin 2012/10/16 AM 11:01 修改僅一般交易跟卡號查詢會回傳 Wave Card Indicator */
	if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
#if 0 //先刪除		
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
			memcpy(&szDataBuffer[inPacketSizes], "P", 1);
		} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
			memcpy(&szDataBuffer[inPacketSizes], "Z", 1);
		} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
		{
			memcpy(&szDataBuffer[inPacketSizes], "G", 1);
		} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_HAPPYCASH_)
		{
			memcpy(&szDataBuffer[inPacketSizes], "H", 1);
		} else
#endif			
		{
			memcpy(&szDataBuffer[inPacketSizes], "O", 1);
		}
	} else
	{
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2))
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			{
				/* 規格未寫M 跟單機同步 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				inGetCardLabel(szTemplate);

				if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
					memcpy(&szDataBuffer[inPacketSizes], "V", 1);
				else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
					memcpy(&szDataBuffer[inPacketSizes], "M", 1);
				else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
					memcpy(&szDataBuffer[inPacketSizes], "J", 1);
				else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
					memcpy(&szDataBuffer[inPacketSizes], "C", 1);
				else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
					memcpy(&szDataBuffer[inPacketSizes], "A", 1);
				else
					memcpy(&szDataBuffer[inPacketSizes], "O", 1);
			}
		}
	}
	inPacketSizes++;

	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);

	inPacketSizes += 4;

	/* Merchant ID (15 Byte) & Terminal ID (8 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2))
	{
		inPacketSizes += 15;
		inPacketSizes += 8;
	} else
	{
		/* Merchant ID (15 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetMerchantID(szTemplate);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PAD_LEFT_FILL_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 15);
		inPacketSizes += 15;
		/* Terminal ID (8 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetTerminalID(szTemplate);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PAD_LEFT_FILL_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
		inPacketSizes += 8;
	}

	/* Exp Amount (12 Byte) */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	inGetTRTFileName(szTemplate);
	if (!memcmp(szTemplate, "DCCTRT", 6) && !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
	{
		if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "0", 1))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%010ld00", atol(pobTran->srBRec.szDCC_TIPFCA));
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		} else if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "1", 1))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%011ld0", atol(pobTran->srBRec.szDCC_TIPFCA));
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		} else if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "2", 1))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%012ld", atol(pobTran->srBRec.szDCC_TIPFCA));
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		}
	} else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTipTxnAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}

	inPacketSizes += 12;

	/* Store Id (18 Byte) */
	inPacketSizes += 18;

	/* 處理紅利、分期 */
	/* Installment / Redeem Indictor (1 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2))
	{
		if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szInstallmentIndicator[0], 1);
		else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szRedeemIndicator[0], 1);
	}

	/* 優惠兌換方式 */
	/* 1.條碼兌換 2.卡號兌換 */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
	{
		if (!memcmp(pobTran->szL3_AwardWay, "1", 1) ||
				!memcmp(pobTran->szL3_AwardWay, "2", 1) ||
				!memcmp(pobTran->szL3_AwardWay, "3", 1))
		{
			memcpy(&szDataBuffer[inPacketSizes], "1", 1);
		} else if (!memcmp(pobTran->szL3_AwardWay, "4", 1) ||
				!memcmp(pobTran->szL3_AwardWay, "5", 1))
		{
			memcpy(&szDataBuffer[inPacketSizes], "2", 1);
		}
	} else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2))
	{
		/* (1= 條碼兌換， 2= 卡號兌換 )*/
		/* 取消優惠兌換只接受 條碼兌換 。 */
		memcpy(&szDataBuffer[inPacketSizes], "1", 1);
	}
	inPacketSizes++;

	/* 處理紅利扣抵 */
	if ((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)) && pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 支付金額 RDM Paid Amt (12 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
		/* 紅利扣抵點數 RDM Point (8 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%08lu", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
		inPacketSizes += 8;
		/* 紅利剩餘點數 Points Of Balance (8 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%08lu", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
		inPacketSizes += 8;
		/* 紅利扣抵金額 Redeem Amt (12 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
	} else
		inPacketSizes += 40;

	/* 處理分期交易 */
	if ((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)) && pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 分期期數 Installment Period (2 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);
		inPacketSizes += 2;
		/* 首期金額 Down Payment (12 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
		/* 每期金額 Installment Payment Amt (12 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
		/* 分期手續費 Formlity Fee (12 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentFormalityFee);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
	} else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		inPacketSizes += 2;

		/* 交易前餘額 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
		{
			if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
				sprintf(szTemplate, "-%09lu00", (unsigned long) (pobTran->srTRec.lnTotalTopUpAmount + (pobTran->srTRec.lnFinalBeforeAmt - 100000)));
			else
				sprintf(szTemplate, "+%09lu00", (unsigned long) (pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTotalTopUpAmount));
		} else
		{
			if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
				sprintf(szTemplate, "-%09lu00", (unsigned long) (pobTran->srTRec.lnFinalBeforeAmt - 100000));
			else
				sprintf(szTemplate, "+%09lu00", (unsigned long) (pobTran->srTRec.lnFinalBeforeAmt));
		}

		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		inPacketSizes += 12;

		/* 交易後餘額 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		if (pobTran->srTRec.lnFinalAfterAmt > 100000)
			sprintf(szTemplate, "-%09lu00", (unsigned long) (pobTran->srTRec.lnFinalAfterAmt - 100000));
		else
			sprintf(szTemplate, "+%09lu00", (unsigned long) (pobTran->srTRec.lnFinalAfterAmt));

		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		inPacketSizes += 12;

		/* 自動加值金額 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
				sprintf(szTemplate, "+%09lu00", (unsigned long) (pobTran->srTRec.lnTotalTopUpAmount));
			else
				sprintf(szTemplate, "+%09lu00", (unsigned long) (0));

			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		}
		inPacketSizes += 12;
	} else
		inPacketSizes += 38;

	/* Card Type (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) && pobTran->srBRec.uszHappyGoSingle != VS_TRUE) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)) && pobTran->srBRec.uszHappyGoMulti == VS_TRUE))
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
#if 0 //先刪除
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_IPASS_, 2);
			} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_ECC_, 2);
			} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_ICASH_, 2);
			} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_HAPPYCASH_)
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_HAPPYCASH_, 2);
			} else
#endif				
				memcpy(&szDataBuffer[inPacketSizes], "FF", 2);
		} else
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			inGetCardLabel(szTemplate);

			if (!memcmp(szTemplate, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_UCARD_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_VISA_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_MASTERCARD_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_JCB_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_AMEX_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_CUP_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_DINERS_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_SMARTPAY_, strlen(_CARD_TYPE_SMARTPAY_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_SMARTPAY_, 2);
			else
				memcpy(&szDataBuffer[inPacketSizes], "FF", 2);
		}
	}

	inPacketSizes += 2;

	/* Batch No (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetBatchNum(szTemplate);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Start Trans Type (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2))
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szStartTransType, 2);

	inPacketSizes += 2;

	/* MP Flag(是否小額交易，MPAS小額交易用) (1 Byte) */
	/* 合併MPAS ECR到標準400 */
	memset(szFESMode, 0x00, sizeof (szFESMode));
	inGetNCCCFESMode(szFESMode);
	if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
	{
		if (pobTran->srBRec.uszMPASTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "M", 1);
		else
			memcpy(&szDataBuffer[inPacketSizes], " ", 1);
	}
	inPacketSizes += 1;


	memset(szTemplate, 0x00, sizeof (szTemplate));
	inGetPayItemEnable(szTemplate);
	if (memcmp(szTemplate, "Y", strlen("Y")) == 0 &&
			(strlen(pobTran->srBRec.szPayItemCode) > 0))
	{
		/* SmartPay要回傳這三項資訊 (99 Bytes) */
		if (!memcmp(&srECROb->srTransData.szCUPIndicator[0], "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/* SP ISSUER ID (8 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscIssuerID[0], 8);
			inPacketSizes += 8;
			/* SP Origin Date (8 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscRefundDate[0], 8);
			inPacketSizes += 8;
			/* SP RRN (12 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscRRN[0], 12);
			inPacketSizes += 12;
		} else
		{
			inPacketSizes += 8;
			inPacketSizes += 8;
			inPacketSizes += 12;
		}

		/* Pay Item (5 Bytes) */
		/* ECR一段式收銀機連線 送空白payitem 不必回傳 */
		if (srECROb->srTransData.uszECRResponsePayitem == VS_FALSE)
		{
			inPacketSizes += 5;
		} else
		{
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szPayItemCode[0], 5);
			inPacketSizes += 5;
		}

		/* 【需求單 - 105039】信用卡為電子發票載具，端末機將卡號加密後回傳收銀機 add by LingHsiung 2016-04-20 上午 09:56:24 */
		/*
			步驟1. Hash Data = 卡號 (U Card則取11碼卡號進行HASH)
			步驟2. Hash Value(32Bytes) = 以SHA-256演算Hash Data
			步驟3. BASE64 Data(44 Bytes) = 將Hash Value以BASE64進行編碼
			步驟4. 加密卡號(50 Bytes) = BIN N0(6 Bytes) + BASE64 Data(44 Bytes)。
			步驟5. 將加密卡號(50 Bytes)依電文規格指定欄位(37)回傳收銀機。
		 */
		/* Card No. Hash Value (50 Bytes) */
		memset(szTemplate, 0x00, sizeof (szTemplate));

		if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
			memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[4], 11);
		else
			memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));

		if (strlen(szTemplate) > 0)
		{
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;
			memset(szHash, 0x00, sizeof (szHash));
			inFunc_CardNumber_Hash(szTemplate, szHash);
			memcpy(&szDataBuffer[inPacketSizes], &szHash[0], 44);
			inPacketSizes += 44;
		} else
		{
			inPacketSizes += 50;
		}

		/* MP Response Code (6 Bytes) */
		memset(szFESMode, 0x00, sizeof (szFESMode));
		inGetNCCCFESMode(szFESMode);
		if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
		{
			if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0 &&
					memcmp(pobTran->srBRec.szRespCode, "11", 2) != 0 &&
					pobTran->srBRec.uszMPASTransBit == VS_TRUE &&
					(memcmp(&pobTran->srBRec.szMPASAuthCode[0], "CAF", 3) == 0 ||
					memcmp(&pobTran->srBRec.szMPASAuthCode[0], "NEG", 3) == 0 ||
					memcmp(&pobTran->srBRec.szMPASAuthCode[0], "WEB", 3) == 0 ||
					memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJ", 3) == 0))
			{
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMPASAuthCode[0], 6);
			}
		}
		inPacketSizes += 6;

		/* ASM award flag */
		inPacketSizes += 1;

		/* 【需求單 - 106128】行動支付標記及金融機構代碼 add by LingHsiung 2017-08-23 上午 11:48:57 */
		if (strlen(pobTran->srBRec.szMCP_BANKID) > 0)
		{
			/* MCP Indicator (1 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[0], 1);
			inPacketSizes += 1;

			/* 金融機構代碼 (3 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[1], 3);
			inPacketSizes += 3;
		}

		/* Reserve (5 Bytes) */
		inPacketSizes += 5;
	} else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		inPacketSizes += 8;
		inPacketSizes += 8;

		/* RF序號 */
		memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTRec.szTicketRefundCode[0], 6);
		inPacketSizes += 12;

		inPacketSizes += 71;
	} else
	{
		/* SmartPay要回傳這三項資訊 (99 Bytes) */
		if (!memcmp(&srECROb->srTransData.szCUPIndicator[0], "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/* SP ISSUER ID (8 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscIssuerID[0], 8);
			inPacketSizes += 8;
			/* SP Origin Date (8 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscRefundDate[0], 8);
			inPacketSizes += 8;
			/* SP RRN (12 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscRRN[0], 12);
			inPacketSizes += 12;
			/* Payitem (5 Byte) */
			inPacketSizes += 5;
			/* 【需求單 - 105039】信用卡為電子發票載具，端末機將卡號加密後回傳收銀機 add by LingHsiung 2016-04-20 上午 09:56:24 */
			/* Card No. Hash Value (50 Bytes) */
			memset(szTemplate, 0x00, sizeof (szTemplate));

			if (memcmp(&pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
				memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[4], 11);
			else
				memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));


			if (strlen(szTemplate) > 0)
			{
				memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
				inPacketSizes += 6;

				memset(szHash, 0x00, sizeof (szHash));
				inFunc_CardNumber_Hash(szTemplate, szHash);
				memcpy(&szDataBuffer[inPacketSizes], &szHash[0], 44);
				inPacketSizes += 44;
			} else
			{
				inPacketSizes += 50;
			}

			/* MP Response Code (6 Byte) */
			memset(szFESMode, 0x00, sizeof (szFESMode));
			inGetNCCCFESMode(szFESMode);
			if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
			{
				if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0 &&
						memcmp(pobTran->srBRec.szRespCode, "11", 2) != 0 &&
						pobTran->srBRec.uszMPASTransBit == VS_TRUE &&
						(memcmp(&pobTran->srBRec.szMPASAuthCode[0], "CAF", 3) == 0 ||
						memcmp(&pobTran->srBRec.szMPASAuthCode[0], "NEG", 3) == 0 ||
						memcmp(&pobTran->srBRec.szMPASAuthCode[0], "WEB", 3) == 0 ||
						memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJ", 3) == 0))
				{
					memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMPASAuthCode[0], 6);
				}
			}
			inPacketSizes += 6;

			/* ASM award flag */
			inPacketSizes += 1;

			/* 【需求單 - 106128】行動支付標記及金融機構代碼 */
			if (strlen(pobTran->srBRec.szMCP_BANKID) > 0)
			{
				/* MCP Indicator (1 Bytes) */
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[0], 1);
				inPacketSizes += 1;

				/* 金融機構代碼 (3 Bytes) */
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[1], 3);
				inPacketSizes += 3;
			} else
			{
				inPacketSizes += 4;
			}

			/* Reserve (5 Bytes) */
			inPacketSizes += 5;
		} else
		{
			/* SP ISSUER ID (8 Byte) */
			inPacketSizes += 8;
			/* SP Origin Date (8 Byte) */
			inPacketSizes += 8;
			/* SP RRN (12 Byte) */
			inPacketSizes += 12;
			/* Payitem (5 Byte) */
			inPacketSizes += 5;
			/* 【需求單 - 105039】信用卡為電子發票載具，端末機將卡號加密後回傳收銀機 add by LingHsiung 2016-04-20 上午 09:56:24 */
			/* Card No. Hash Value (50 Bytes) */
			memset(szTemplate, 0x00, sizeof (szTemplate));

			if (memcmp(&pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
				memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[4], 11);
			else
				memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));


			if (strlen(szTemplate) > 0)
			{
				memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
				inPacketSizes += 6;

				memset(szHash, 0x00, sizeof (szHash));
				inFunc_CardNumber_Hash(szTemplate, szHash);
				memcpy(&szDataBuffer[inPacketSizes], &szHash[0], 44);
				inPacketSizes += 44;
			} else
			{
				inPacketSizes += 50;
			}

			/* MP Response Code (6 Byte) */
			memset(szFESMode, 0x00, sizeof (szFESMode));
			inGetNCCCFESMode(szFESMode);
			if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
			{
				if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0 &&
						memcmp(pobTran->srBRec.szRespCode, "11", 2) != 0 &&
						pobTran->srBRec.uszMPASTransBit == VS_TRUE &&
						(memcmp(&pobTran->srBRec.szMPASAuthCode[0], "CAF", 3) == 0 ||
						memcmp(&pobTran->srBRec.szMPASAuthCode[0], "NEG", 3) == 0 ||
						memcmp(&pobTran->srBRec.szMPASAuthCode[0], "WEB", 3) == 0 ||
						memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJ", 3) == 0))
				{
					memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMPASAuthCode[0], 6);
				}
			}
			inPacketSizes += 6;

			/* ASM award flag */
			if ((pobTran->srBRec.uszRewardL1Bit == VS_TRUE ||
					pobTran->srBRec.uszRewardL2Bit == VS_TRUE ||
					pobTran->srBRec.uszRewardL5Bit == VS_TRUE) &&
					pobTran->srBRec.uszRewardSuspendBit != VS_TRUE)
			{
				memcpy(&szDataBuffer[inPacketSizes], "A", strlen("A"));
			} else
			{
				memcpy(&szDataBuffer[inPacketSizes], " ", strlen(" "));
			}
			inPacketSizes += 1;

			/* 【需求單 - 106128】行動支付標記及金融機構代碼 */
			if (strlen(pobTran->srBRec.szMCP_BANKID) > 0)
			{
				/* MCP Indicator (1 Bytes) */
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[0], 1);
				inPacketSizes += 1;

				/* 金融機構代碼 (3 Bytes) */
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[1], 3);
				inPacketSizes += 3;
			} else
			{
				inPacketSizes += 4;
			}

			/* Reserve (5 Bytes) */
			inPacketSizes += 5;
		}
	}

	/* HG Data (78 Byte) */
	if (pobTran->srBRec.lnHGTransactionType != 0 &&
			(pobTran->srBRec.uszHappyGoMulti == VS_TRUE || pobTran->srBRec.uszHappyGoSingle == VS_TRUE))
	{
		/* Payment Tools (2 Byte) */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2))
		{
			if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_)
				memcpy(&szDataBuffer[inPacketSizes], "01", 2);
			else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_)
				memcpy(&szDataBuffer[inPacketSizes], "02", 2);
			else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_)
				memcpy(&szDataBuffer[inPacketSizes], "03", 2);
			else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_INSIDE_)
				memcpy(&szDataBuffer[inPacketSizes], "04", 2);
			else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CUP_)
				memcpy(&szDataBuffer[inPacketSizes], "05", 2);
		}

		inPacketSizes += 2;

		/* HG Card Number (18 Byte) */
		/* 重印HG混信用卡的交易簽單不回傳HG卡號 */
		if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2) != 0)
		{
			inRetVal = inECR_CardNoTruncateDecision_HG(pobTran);
			/* 要遮卡號 */
			if (inRetVal == VS_SUCCESS)
			{
				memset(szTemplate, 0x00, sizeof (szTemplate));
				strcpy(szTemplate, pobTran->srBRec.szHGPAN);

				/* HAPPG_GO 卡不掩飾 */
				if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
				{

				} else
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
						for (i = 2; i < (inCardLen - 4); i++)
							szTemplate[i] = '*';
					} else
					{
						for (i = 6; i < (inCardLen - 4); i++)
							szTemplate[i] = '*';
					}

				}

				memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
			} else
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szHGPAN, strlen(pobTran->srBRec.szHGPAN));
			}

		}

		inPacketSizes += 18;

		/* HG Pay Amount (12 Byte) 實際支付金額 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnHGAmount);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		}

		inPacketSizes += 12;
		/* HG Redeem Amount (12 Byte) 扣抵金額 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnHGRedeemAmount);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		}

		inPacketSizes += 12;
		/* HG Redeem Point (8 Byte) 扣抵點數 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGTransactionPoint);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
		}

		inPacketSizes += 8;
		/* HG Lack Point (8 Byte) 不足點數*/
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGRefundLackPoint);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
		}

		inPacketSizes += 8;
		/* HG Balance Point (8 Byte) 剩餘點數 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGBalancePoint);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
		}

		inPacketSizes += 8;
		/* HG Reserve (10 Byte) */
		inPacketSizes += 10;
	} else
		inPacketSizes += 78;

	return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_Standard_Pack_ResponseCode
Date&Time       :2016/7/6 下午 2:40
Describe        :先把要送的資料組好
 */
int inECR_8N1_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	char szTRTFileName[12 + 1];
	char szFESMode[2 + 1];

	/* 非參加機構卡片判斷 */
	if (!memcmp(pobTran->srBRec.szRespCode, "05", 2) && (!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJB01", 6) ||
			!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJB02", 6) ||
			!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJB03", 6)))
	{
		srECROb->srTransData.inErrorType = _ECR_RESPONSE_CODE_NOT_MEMBER_CARD_;
	}

	if (srECROb->srTransData.inErrorType == VS_CALLBANK)
	{
		memcpy(&szDataBuf[76], "0002", 4);
	} else if (srECROb->srTransData.inErrorType == VS_TIMEOUT ||
			srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_)
	{
		memcpy(&szDataBuf[76], "0003", 4);
	} else if (srECROb->srTransData.inErrorType == VS_USER_OPER_ERR || /* 操作錯誤 & 交易流程有誤 */
			srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_)
	{
		memcpy(&szDataBuf[76], "0004", 4);
	} else if (srECROb->srTransData.inErrorType == VS_ISO_PACK_ERR ||
			srECROb->srTransData.inErrorType == VS_ISO_UNPACK_ERROR ||
			srECROb->srTransData.inErrorType == VS_COMM_ERROR ||
			srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COMM_ERROR_)
	{
		memcpy(&szDataBuf[76], "0005", 4);
	} else if (srECROb->srTransData.inErrorType == VS_USER_CANCEL ||
			srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_)
	{
		memcpy(&szDataBuf[76], "0006", 4);
	} else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_NOT_MEMBER_CARD_)
	{
		memcpy(&szDataBuf[76], "0009", 4); /* 非參加機構卡片 */
	} else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_)
	{
		memcpy(&szDataBuf[76], "0000", 4);
	} else
	{
		memset(szTRTFileName, 0x00, sizeof (szTRTFileName));
		inGetTRTFileName(szTRTFileName);
		/* 2017/9/6 上午 11:21 看verifone code新增1301 */
		if (memcmp(&pobTran->srBRec.szRespCode[0], "00", 2) &&
				memcmp(&pobTran->srBRec.szRespCode[0], "11", 2) &&
				memcmp(szTRTFileName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
		{
			memcpy(&szDataBuf[76], "1301", 4);
		} else
		{
			memcpy(&szDataBuf[76], "0001", 4);
		}
	}

	memset(szFESMode, 0x00, sizeof (szFESMode));
	inGetNCCCFESMode(szFESMode);
	if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)))
	{
		if (pobTran->srBRec.uszMPASTransBit == VS_TRUE)
			memcpy(&szDataBuf[222], "M", 1);
		else
			memcpy(&szDataBuf[222], " ", 1);

		/* MP Response Code */
		if ((memcmp(pobTran->srBRec.szRespCode, "00", 2)) &&
				(!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "CAF", 3) || !memcmp(&pobTran->srBRec.szMPASAuthCode[0], "NEG", 3) ||
				!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "WEB", 3) || !memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJ", 3)))
		{
			memcpy(&szDataBuf[306], &pobTran->srBRec.szMPASAuthCode[0], 6);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Standard_Unpack
Date&Time       :2017/11/16 上午 10:45
Describe        :分析收銀機傳來的資料
 */
int inECR_7E1_Standard_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int i = 0;
	int inRetVal = VS_SUCCESS;
	int inTransType = 0; /* 電文中的交易別字串轉為數字儲存 */
	int inHGPaymetType = 0;
	char szTemplate[100 + 1] = {0};
	char szDebugMsg[100 + 1] = {0};
	char szHostLabel[8 + 1] = {0};
	char szHostEnable[2 + 1] = {0};
	char szBatchNum[6 + 1] = {0};
	char szCTLSEnable[2 + 1] = {0};

	/* Trans Type (交易別) */
	switch (inTransType)
	{
		default:
			memset(srECROb->srTransData.szTransType, 0x00, sizeof (srECROb->srTransData.szTransType));
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[0], 2);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			} else
			{
				memcpy(srECROb->srTransData.szTransType, szTemplate, 2);
			}

			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);

			/* 要連動結帳 */
			if (inTransType == _ECR_7E1_SETTLEMENT_NO_)
			{
				pobTran->uszAutoSettleBit = VS_TRUE;
			}

			break;
	}

	/* 銀行別/付款別 HOST ID*/
	switch (inTransType)
	{
		case _ECR_7E1_VOID_NO_: /* 取消交易 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[2], 2);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			}

			memset(szHostLabel, 0x00, sizeof (szHostLabel));

			if (memcmp(szTemplate, _ECR_7E1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_));
			} else if (memcmp(szTemplate, _ECR_7E1_NCCC_HOSTID_DCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
			}				/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			else
			{
				return (VS_ERROR);
			}

			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					/* 如果return VS_ERROR 代表table沒有該HOST */
					inDISP_LogPrintfWithFlag(" ECR 7E1 Standard Unpack Load HDT[%d] *Error* Line[%d]", i, __LINE__);
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof (szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof (szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inDISP_LogPrintf(szDebugMsg);
						}

						/* 如果return VS_ERROR 代表Host沒開 */
						return (VS_ERROR);
					} else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;

						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof (szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inDISP_LogPrintf(szDebugMsg);
						}

					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;

		default:
			break;
	}

	/* EDC簽單序號 Receipt No(InvoiceNumber) */
	switch (inTransType)
	{
		case _ECR_7E1_VOID_NO_: /* 取消交易 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(&szTemplate[0], &szDataBuffer[4], 6);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			} else
			{
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;
		default:
			break;
	}

	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(&szTemplate[0], &szDataBuffer[33], 10);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			} else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inDISP_LogPrintf(szDebugMsg);
			}
			break;
	}

	/* 交易時間 Trans Time */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[51], 6);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Time", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				/* 有時候會不送時間 */
				//				return (VS_ERROR);
			} else
			{
				sprintf(pobTran->srBRec.szTime, "%s", szTemplate);
				sprintf(pobTran->srBRec.szOrgTime, "%s", szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易時間", pobTran->srBRec.szTime);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;


			break;
	}

	/* 授權碼(Auth Code) ECR電文中給9個字元(左靠右補空白) 但ATS電文只需要6個，所以只抓6個(交易補登及所有退貨須送此欄位) */
	switch (inTransType)
	{
		case _ECR_7E1_REFUND_NO_: /* 退貨 */
		case _ECR_7E1_OFFLINE_NO_: /* 交易補登 */
		case _ECR_7E1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
		case _ECR_7E1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_7E1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof (pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[57], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);

			break;

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Code", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			} else
			{
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}

			break;

		default:
			break;
	}

	/* szDataBuffer[66] 144規格要看紅利分期來看使用用途  */
	switch (inTransType)
	{
		case _ECR_7E1_PREAUTH_NO_: /* 預先授權完成(用來放原預先授權金額) */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[66], 10);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Amount", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			}
			/* 預授金額 */
			pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
			break;
		case _ECR_7E1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_7E1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[66], 10); /* 12位數，這裡只取10位 */

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "RDM Paid Amt", szTemplate);
				inDISP_LogPrintf(szDebugMsg);
			}
			/* 實際支付金額 */
			pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);

			break;
		case _ECR_7E1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[200], 10); /* 12位數，這裡只取10位 */

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
				inDISP_LogPrintf(szDebugMsg);
			}
			pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

			if (pobTran->srBRec.lnInstallmentFormalityFee != 0)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_;

				return (VS_ERROR);
			}

			break;
		default:
			break;
	}

	/* 分期期數 Installment Period */
	switch (inTransType)
	{
		case _ECR_7E1_INSTALLMENT_NO_: /* 分期 */
		case _ECR_7E1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
		case _ECR_7E1_HG_REWARD_INSTALLMENT_NO_: /* HG紅利積點(分期)*/
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[66], 2);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Installment Period", szTemplate);
				inDISP_LogPrintf(szDebugMsg);
			}
			/* 分期期數(右靠左補0) */
			pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);

			break;

		default:
			break;
	}

	/* szDataBuffer[102] 144規格要看紅利分期來看使用用途 保留欄位/首期金額／紅利扣抵點數  */
	switch (inTransType)
	{
		case _ECR_7E1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_7E1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
			/* 最多八位 所以從106開始 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[106], 8);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "RDM Point", szTemplate);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 信用卡紅利扣抵點數 RDM Point */
			pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);
			break;
		case _ECR_7E1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[102], 10); /* 12位數，這裡只取10位 */

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Down Payment Amount", szTemplate);
				inDISP_LogPrintf(szDebugMsg);
			}
			/* 首期金額 Down Payment Amount (含小數 2位) */
			pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);

			break;
		case _ECR_7E1_HG_POINT_CERTAIN_NO_: /* 加價購*/
		case _ECR_7E1_HG_FULL_REDEEMPTION_NO_: /* 點數兌換 */
		case _ECR_7E1_HG_REDEEM_REFUND_NO_: /* 扣抵退貨 */
		case _ECR_7E1_HG_REWARD_REFUND_NO_: /* 回饋退貨 */
			/* 最多八位 所以從106開始 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[106], 8);
			pobTran->srBRec.lnHGTransactionPoint = atol(szTemplate);
			break;
		default:
			break;
	}



	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[114], 18);

			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inDISP_LogPrintf(szDebugMsg);
			}
			break;
	}

	/* szDataBuffer[132] 144規格要看紅利分期來看使用用途  卡號查詢/每期金額/保留欄位 */
	switch (inTransType)
	{
		case _ECR_7E1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[132], 10); /* 12位數，這裡只取10位 */

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Payment Amount", szTemplate);
				inDISP_LogPrintf(szDebugMsg);
			}
			/* 每期金額 Installment Payment Amount (含小數 2位) */
			pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);

			break;
		default:
			break;
	}

	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
	}

	/* szDataBuffer[142] 144規格要看紅利分期來看使用用途  卡號查詢/每期金額/保留欄位 */
	switch (inTransType)
	{
			/* 啟動卡號查詢 Start Trans Type */
		case _ECR_7E1_START_CARD_NO_INQUIRY_NO_: /* 卡號查詢 */
			/* 卡號查詢 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(&srECROb->srTransData.szStartTransType[0], &szDataBuffer[142], 2);
			memcpy(&szTemplate[0], &szDataBuffer[142], 2);
			inTransType = atoi(szTemplate);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Start Trans Type", szTemplate);
				inDISP_LogPrintf(szDebugMsg);
			}

			switch (inTransType)
			{
				case _ECR_7E1_SALE_NO_:
				case _ECR_7E1_REFUND_NO_:
				case _ECR_7E1_PREAUTH_NO_:
					if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
					{

					} else
					{
						pobTran->uszCardInquiryFirstBit = VS_TRUE;
					}
					/* 有值影響到後面判斷  確保都是空格 */
					memcpy(&srECROb->srTransData.szCUPIndicator[0], " ", 1);
					memcpy(&srECROb->srTransData.szTransIndicator[0], " ", 1);
					break;
				case _ECR_7E1_OFFLINE_NO_:
				case _ECR_7E1_INSTALLMENT_NO_:
				case _ECR_7E1_REDEEM_NO_:
				case _ECR_7E1_INSTALLMENT_REFUND_NO_:
				case _ECR_7E1_REDEEM_REFUND_NO_:
				case _ECR_7E1_INSTALLMENT_ADJUST_NO_:
				case _ECR_7E1_REDEEM_ADJUST_NO_:
					if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
					{

					} else
					{
						pobTran->uszCardInquiryFirstBit = VS_TRUE;
					}
					memcpy(&srECROb->srTransData.szCUPIndicator[0], " ", 1);
					memcpy(&srECROb->srTransData.szTransIndicator[0], " ", 1);
					break;
				default:
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
					return (VS_ERROR);
			}
			break;
		case _ECR_7E1_HG_REWARD_SALE_NO_: /* HG紅利積點*/
		case _ECR_7E1_HG_REWARD_REDEMPTION_NO_: /* HG紅利積點(紅利)*/
		case _ECR_7E1_HG_REWARD_INSTALLMENT_NO_: /* HG紅利積點(分期)*/
		case _ECR_7E1_HG_POINT_CERTAIN_NO_: /* HG加價購 */
		case _ECR_7E1_HG_ONLINE_REDEEMPTION_NO_: /* HG點數扣抵 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[142], 2);
			inHGPaymetType = atoi(szTemplate);
			if (inHGPaymetType == 1)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CREDIT_;
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("_HG_PAY_CREDIT_");
				}
			} else if (inHGPaymetType == 2)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CASH_;
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("_HG_PAY_CASH_");
				}
			} else if (inHGPaymetType == 3)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_GIFT_PAPER_;
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("_HG_PAY_GIFT_PAPER_");
				}
			} else if (inHGPaymetType == 4)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CREDIT_INSIDE_;
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("_HG_PAY_CREDIT_INSIDE_");
				}
			} else if (inHGPaymetType == 5)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CUP_;
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("_HG_PAY_CUP_");
				}
			} else
			{
				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf("HG PaymentType Error, %d", inHGPaymetType);
				}
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				return (VS_ERROR);
			}
			break;
		default:
			break;
	}

	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
				pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;

				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜銀聯一般交易＞ */

				pobTran->inFunctionID = _CUP_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = _CUP_SALE_;
				pobTran->srBRec.inOrgCode = _CUP_SALE_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
				} else
				{
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
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

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}

			break;
		case _ECR_7E1_VOID_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜沖正交易＞ */

				pobTran->inFunctionID = _FISC_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_FISC_VOID_;

				pobTran->inTransactionCode = _FISC_VOID_;
				pobTran->srBRec.inCode = _FISC_VOID_;
				pobTran->srBRec.inOrgCode = _FISC_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = _CUP_VOID_;
				pobTran->srBRec.inOrgCode = _CUP_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_7E1_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _CUP_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
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

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜退貨交易＞ */

				pobTran->inFunctionID = _TICKET_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_REFUND_;
				} else
				{
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_REFUND_;
				pobTran->srTRec.inCode = _TICKET_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
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

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_7E1_OFFLINE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜交易補登＞ */

			pobTran->inFunctionID = _SALE_OFFLINE_;
			pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

			pobTran->inTransactionCode = _SALE_OFFLINE_;
			pobTran->srBRec.inCode = _SALE_OFFLINE_;
			pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_7E1_PREAUTH_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權＞ */

				pobTran->inFunctionID = _PRE_AUTH_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inTransactionCode = _PRE_AUTH_;
				pobTran->srBRec.inCode = _PRE_AUTH_;
				pobTran->srBRec.inOrgCode = _PRE_AUTH_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權＞ */

				pobTran->inFunctionID = _PRE_AUTH_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inTransactionCode = _PRE_AUTH_;
				pobTran->srBRec.inCode = _PRE_AUTH_;
				pobTran->srBRec.inOrgCode = _PRE_AUTH_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}

			break;

		case _ECR_7E1_INSTALLMENT_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 保留給銀聯分期使用 */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_7E1_INSTALLMENT_REFUND_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 保留給銀聯分期退貨使用 */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_7E1_INSTALLMENT_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期調帳＞ */

			pobTran->inFunctionID = _INST_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_INST_ADJUST_;
			pobTran->inRunTRTID = _TRT_INST_ADJUST_;

			pobTran->inTransactionCode = _INST_ADJUST_;
			pobTran->srBRec.inCode = _INST_ADJUST_;
			pobTran->srBRec.inOrgCode = _INST_ADJUST_;

			pobTran->srBRec.uszInstallmentBit = VS_TRUE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_7E1_REDEEM_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 保留給銀聯紅利扣抵使用 */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_7E1_REDEEM_REFUND_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 保留給銀聯紅利退貨使用 */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_CUP_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_7E1_REDEEM_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利調帳＞ */

			pobTran->inFunctionID = _REDEEM_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_REDEEM_ADJUST_;
			pobTran->inRunTRTID = _TRT_REDEEM_ADJUST_;

			pobTran->inTransactionCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inOrgCode = _REDEEM_ADJUST_;

			pobTran->srBRec.uszRedeemBit = VS_TRUE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購紅利積點 */
		case _ECR_7E1_HG_REWARD_SALE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡紅利積點＞ */

			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_CREDIT_CTLS_;

			/* 這邊先設成sale，若之後支付方式選銀聯卡，會在那邊改成CUP_SALE */
			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購紅利積點─分期付款 */
		case _ECR_7E1_HG_REWARD_INSTALLMENT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡分期付款＞ */

			pobTran->inFunctionID = _INST_SALE_;
			pobTran->inRunOperationID = _OPERATION_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_INSTALLMENT_CTLS_;

			pobTran->inTransactionCode = _INST_SALE_;
			pobTran->srBRec.inCode = _INST_SALE_;
			pobTran->srBRec.inOrgCode = _INST_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購紅利積點─紅利扣抵 */
		case _ECR_7E1_HG_REWARD_REDEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡紅利扣抵＞ */

			pobTran->inFunctionID = _REDEEM_SALE_;
			pobTran->inRunOperationID = _OPERATION_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REDEMPTION_CTLS_;

			pobTran->inTransactionCode = _REDEEM_SALE_;
			pobTran->srBRec.inCode = _REDEEM_SALE_;
			pobTran->srBRec.inOrgCode = _REDEEM_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszRedeemBit = VS_TRUE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購點數扣抵 */
		case _ECR_7E1_HG_ONLINE_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡點數扣抵＞ */

			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_ONLINE_REDEEM_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_ONLINE_REDEEM_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購加價購 */
		case _ECR_7E1_HG_POINT_CERTAIN_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡加價購＞ */

			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_POINT_CERTAIN_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_POINT_CERTAIN_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購點數兌換 */
		case _ECR_7E1_HG_FULL_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡點數兌換＞ */

			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_FULL_REDEMPTION_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_FULL_REDEMPTION_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購扣抵退貨 */
		case _ECR_7E1_HG_REDEEM_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡扣抵退貨＞ */

			pobTran->inFunctionID = _HG_REDEEM_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REDEEM_REFUND_;

			pobTran->inTransactionCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REDEEM_REFUND_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購回饋退貨 */
		case _ECR_7E1_HG_REWARD_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡回饋退貨＞ */

			pobTran->inFunctionID = _HG_REWARD_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REFUND_;

			pobTran->inTransactionCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_REFUND_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購點數查詢 */
		case _ECR_7E1_HG_POINT_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡點數查詢＞ */

			pobTran->inFunctionID = _HG_INQUIRY_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_INQUIRY_;

			pobTran->inTransactionCode = _HG_INQUIRY_;
			pobTran->srBRec.inCode = _HG_INQUIRY_;
			pobTran->srBRec.inOrgCode = _HG_INQUIRY_;
			pobTran->srBRec.lnHGTransactionType = _HG_INQUIRY_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_7E1_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;

			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_7E1_END_CARD_NO_INQUIRY_NO_:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_;
			inRetVal = VS_ERROR;
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}

	return (inRetVal);
}

/*
Function        :inECR_7E1_Standard_Pack
Date&Time       :2017/11/16 上午 10:45
Describe        :先把要送的資料組好
 */
int inECR_7E1_Standard_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int inRetVal;
	int i, inCardLen;
	int inPacketSizes = 0;
	char szTemplate[100 + 1];

	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;

	inPacketSizes++;

	/* HOST ID (2 Byte) */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	inGetTRTFileName(szTemplate);

	if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_NCCC_, 2);
		/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_DCC_, 2);

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->srBRec.lnOrgInvNum != 0L ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)) /* 點數查詢就算是沒有INV也要回傳 */
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Card No (19 Byte)(左靠右補空白) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2) ||
			pobTran->srBRec.inOrgCode == _HG_REWARD_REFUND_ || /* 取消回饋或扣抵退貨不需回傳信用卡卡號 */
			pobTran->srBRec.inOrgCode == _HG_REDEEM_REFUND_ ||
			pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_ || /* 若是純HG交易，信用卡卡號不需回傳 */
			pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_ ||
			pobTran->srBRec.inHGCode == _HG_FULL_REDEMPTION_) /* 修改ECR交易取消點數兌換多印卡號。*/
	{
		inPacketSizes += 19;
	} else
	{
		/* 卡號是否遮掩 */
		inRetVal = inECR_CardNoTruncateDecision(pobTran);

		/* 要遮卡號 */
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szPAN);

			/* HAPPG_GO 卡不掩飾 */
			if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
			{

			} else
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
					for (i = 2; i < (inCardLen - 4); i++)
						szTemplate[i] = '*';
				} else
				{
					for (i = 6; i < (inCardLen - 4); i++)
						szTemplate[i] = '*';
				}

			}

			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		} else
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
		}

		inPacketSizes += 19;
	}

	/* Card Expire Date (4 Byte) */
	/* 這裡回傳條件抄Verifone的Code */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2) || /* 點數查詢 */
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2) || /* 點數扣抵 */
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2) || /* 扣抵退貨 */
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2) || /* 回饋退貨 */
			pobTran->srBRec.inOrgCode == _HG_REWARD_REFUND_ || /* 取消回饋或扣抵退貨不需回傳信用卡卡號 */
			pobTran->srBRec.inOrgCode == _HG_REDEEM_REFUND_ ||
			pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_ || /* 若是純HG交易，信用卡卡號不需回傳 */
			pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_)
	{
		inPacketSizes += 4;
	} else
	{
		/* 有效期是否回傳 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetECRExpDateReturnEnable(szTemplate);

		/* 要回傳有效期 */
		if (!memcmp(&szTemplate, "Y", 1))
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

		inPacketSizes += 4;
	}

	/* Trans Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_ADJUST_, 2))
	{
		if (pobTran->srBRec.lnTxnAmount != 0)
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
		inPacketSizes += 12;
	}

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REDEMPTION_, 2))
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;

		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
		inPacketSizes += 6;
	} else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (strlen(pobTran->srBRec.szAuthCode) > 0)
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szAuthCode);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PAD_LEFT_FILL_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
	}

	inPacketSizes += 9;

	/* 保留欄位/實際支付金額(紅利交易)/分期期數 + 分期手續費(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 實際支付金額 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

		inPacketSizes += 12;
	} else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 分期期數 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);

		inPacketSizes += 2;

		/* 分期手續費 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010ld", pobTran->srBRec.lnInstallmentFormalityFee);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);

		inPacketSizes += 10;
	} else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_ && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* HG點數扣抵要回傳實際支付金額 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnHGAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

		inPacketSizes += 12;
	} else
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "W", 1);

		inPacketSizes++;

		if (!memcmp(&srECROb->srTransData.szCUPIndicator[0], "C", 1))
			memcpy(&szDataBuffer[inPacketSizes], &srECROb->srTransData.szCUPIndicator[0], 1);
		else
			memcpy(&szDataBuffer[inPacketSizes], "0", 1);

		inPacketSizes++; /* CUP Indicator */
		inPacketSizes += 10;
	}

	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;

	/* Terminal ID (8 Byte) */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PAD_LEFT_FILL_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	/* Reference No */
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szRefNo, 12);
	}
	inPacketSizes += 12;

	/* 保留欄位/扣抵紅利點數(紅利交易)/首期金額(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 實際支付金額 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	} else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 首期金額 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	} else
	{
		/* HG點數扣抵 回傳扣抵點數 */
		if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_ && pobTran->srBRec.uszVOIDBit != VS_TRUE)
			sprintf(szTemplate, "%012lu", pobTran->srBRec.lnHGTransactionPoint);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	inPacketSizes += 12;

	/* HGPAN(Request:Store Id Response:HappyGo Card Number) (18 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REDEMPTION_, 2))
	{
		inRetVal = inECR_CardNoTruncateDecision_HG(pobTran);
		/* 要遮卡號 */
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szHGPAN);

			/* HAPPG_GO 卡不掩飾 */
			if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
			{

			} else
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
					for (i = 2; i < (inCardLen - 4); i++)
						szTemplate[i] = '*';
				} else
				{
					for (i = 6; i < (inCardLen - 4); i++)
						szTemplate[i] = '*';
				}

			}

			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		} else
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szHGPAN, strlen(pobTran->srBRec.szHGPAN));
		}
	}
	inPacketSizes += 18;

	/* 保留欄位(10 Byte)+ START Trans Type (2 Byte) /剩餘紅利點數(紅利交易)/每期金額(分期交易) (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_START_CARD_NO_INQUIRY_, 1))
	{
		inPacketSizes += 10;

		/* START Trans Type (2 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szStartTransType, 2);

		inPacketSizes += 2;
	} else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 剩餘紅利點數(紅利交易) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

		inPacketSizes += 12;
	} else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 每期金額(分期交易) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010ld", pobTran->srBRec.lnInstallmentPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

		inPacketSizes += 12;
	}

	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Standard_Pack_ResponseCode
Date&Time       :2017/11/16 上午 10:49
Describe        :
 */
int inECR_7E1_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	char szTmeplate[12 + 1];

	/* 不知道Verifone是照哪裡的規格，總之照Verifone的Code */
	if (srECROb->srTransData.inErrorType == VS_TIMEOUT)
	{
		/* 刷卡前沒有host id  先傳 03 */
		if (pobTran->srBRec.inHDTIndex == -1)
			memcpy(&szDataBuf[78], "03", 2);
		else
		{
			memset(szTmeplate, 0x00, sizeof (szTmeplate));
			inGetTRTFileName(szTmeplate);
			if (memcmp(szTmeplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
			{
				memcpy(&szDataBuf[78], "04", 2);
			}				/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			else
			{
				memcpy(&szDataBuf[78], "03", 2);
			}

		}

		memcpy(&szDataBuf[80], "0A", 2);
	} else if (memcmp(&pobTran->srBRec.szRespCode[0], "00", 2) &&
			memcmp(&pobTran->srBRec.szRespCode[0], "11", 2)
			)
	{
		if (memcmp(szTmeplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
			memcpy(&szDataBuf[78], "04", 2);
		else if (memcmp(szTmeplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0)
			memcpy(&szDataBuf[78], "03", 2);
		else if (memcmp(szTmeplate, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
			memcpy(&szDataBuf[78], "13", 2);
		/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */

		memcpy(&szDataBuf[80], &pobTran->srBRec.szRespCode[0], 2);
	} else
		memcpy(&szDataBuf[78], "A000", 4);

	//	if (srECROb->srTransData.inErrorType == VS_CALLBANK)
	//		memcpy(&szDataBuf[78], "0002", 4);
	//	else if (srECROb->srTransData.inErrorType == VS_TIMEOUT)
	//		memcpy(&szDataBuf[78], "0003", 4);
	//	else if (srECROb->srTransData.inErrorType == VS_USER_CANCEL)
	//		memcpy(&szDataBuf[78], "0006", 4);
	//	else if (srECROb->srTransData.inErrorType == VS_ISO_PACK_ERR || srECROb->srTransData.inErrorType == VS_ISO_UNPACK_ERROR ||
	//		 srECROb->srTransData.inErrorType == VS_COMM_ERROR)
	//		memcpy(&szDataBuf[78], "0007", 4);
	//	else
	//	{
	//		memcpy(&szDataBuf[78], "0001", 4);
	//	}

	return (VS_SUCCESS);
}

/*
Function        :inECR_LOAD_TMK_FROM_520_Standard_Unpack
Date&Time       :2017/11/16 下午 2:45
Describe        :
 */
int inECR_LOAD_TMK_FROM_520_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int inRetVal = VS_SUCCESS;
	int inCnt = 0;
	int inKeyCnt = 0;
	char szTemplate[100 + 1];

	/* 區分設備 */
	inCnt += 7;

	/* 計算〈TMK KCT〉 */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	memcpy(szTemplate, &szDataBuffer[inCnt], 2);
	inKeyCnt = atoi(szTemplate);
	inCnt += 2;

	/* 取TMK DATA */
	//	inRetVal = inNCCC_TMK_Write_TerminalMasterKey_By_KeyCard(inKeyCnt, &szDataBuffer[inCnt]);


	return (inRetVal);
}

/*
Function        :inECR_LOAD_TMK_FROM_520_Standard_Pack
Date&Time       :2017/11/16 下午 2:46
Describe        :
 */
int inECR_LOAD_TMK_FROM_520_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_Customer_107_Bumper_Unpack
Date&Time       :2017/11/15 下午 2:44
Describe        :分析收銀機傳來的資料
 */
int inECR_8N1_Customer_107_Bumper_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int i = 0;
	int inRetVal = VS_SUCCESS;
	int inTransType = 0; /* 電文中的交易別字串轉為數字儲存 */
	char szTemplate[100 + 1] = {0};
	char szDebugMsg[100 + 1] = {0};
	char szHostLabel[8 + 1] = {0};
	char szHostEnable[2 + 1] = {0};
	char szBatchNum[6 + 1] = {0};
	char szCTLSEnable[2 + 1] = {0};

	/* ECR Indicator :新 ECR連線 Indicator"I"(規格新增欄位 )。 */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[0], 1);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "ECR Indicator", szTemplate);
				inDISP_LogPrintf(szDebugMsg);
			}
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "ECR Indicator", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			}

			/* ECR Indicator必為'I'，否則error */
			if (memcmp(&szTemplate[0], "I", 1) != 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "ECR Indicator", "Not \"I\" Error");
					inDISP_LogPrintf(szDebugMsg);
				}
				return (VS_ERROR);
			}
			break;
	}

	/* Trans Type (交易別) */
	switch (inTransType)
	{
		default:
			memset(srECROb->srTransData.szTransType, 0x00, sizeof (srECROb->srTransData.szTransType));
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[8], 2);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
				inDISP_LogPrintf(szDebugMsg);
			}

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			} else
			{
				memcpy(srECROb->srTransData.szTransType, szTemplate, 2);
			}

			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);

			/* 要連動結帳 */
			if (inTransType == _ECR_8N1_SETTLEMENT_NO_)
			{
				pobTran->uszAutoSettleBit = VS_TRUE;
			}

			break;
	}

	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
	}

	/* 啟動卡號查詢不受Trans Indicator影響 */
	/* 客製化107改為Echo Host */
	if (inTransType == _ECR_8N1_START_CARD_NO_INQUIRY_NO_)
	{
		/* 執行Echo Test */
	} else
	{
		/* Trans Type Indicator :支援交易別 :一般交易、補登退貨 ，其餘 交易別，端末機不參考此欄位 。*/
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_: /* 一般交易 */
			case _ECR_8N1_REFUND_NO_: /* 退貨交易 */
			case _ECR_8N1_OFFLINE_NO_: /* 交易補登 */
				memset(srECROb->srTransData.szTransIndicator, 0x00, sizeof (srECROb->srTransData.szTransIndicator));
				memcpy(srECROb->srTransData.szTransIndicator, &szDataBuffer[7], 1);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type Indicator", srECROb->srTransData.szTransIndicator);
					inDISP_LogPrintf(szDebugMsg);
				}
				/* Trans Type Indicator是'S'，要進選擇畫面 */
				if (!memcmp(&srECROb->srTransData.szTransIndicator[0], "S", 1))
				{
					memset(srECROb->srTransData.szTransIndicator, 0x00, sizeof (srECROb->srTransData.szTransIndicator));
					inRetVal = inRS232_ECR_SelectTransType(srECROb);
					/* 交易類別轉成數字 */
					inTransType = atoi(srECROb->srTransData.szTransType);

					/* Select失敗 */
					if (inRetVal != VS_SUCCESS)
					{
						return (inRetVal);
					}
				}
				break;
			default:
				break;
		}

		/* 要判斷是否空值，有可能Trans Type Indicator就會順便塞值 */
		if (memcmp(&srECROb->srTransData.szCUPIndicator, "\x00", 1) == 0)
		{
			/* CUP/Smart pay Indicator:銀聯卡 / Smart Pay交易使用
			 * Indicator = 'C'，表示為CUP交易
			 * Indicator = 'N'，表示為一般信用卡交易
			 * Indicator = 'S'，表示為SmartPay交易，(SmartPay的Sale Reversal和Refund，收銀機一定要送'S'。SmartPay的Sale送'N'，因為Sale不需要按Hotkey)
			 */
			switch (inTransType)
			{
				case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* 啟動卡號查詢 */
				case _ECR_8N1_SALE_NO_: /* 一般交易 */
				case _ECR_8N1_REFUND_NO_: /* 退貨交易 */
				case _ECR_8N1_VOID_NO_: /* 取消 */
				case _ECR_8N1_PREAUTH_NO_: /* 預先授權 */
				case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 */
				case _ECR_8N1_ESVC_TOP_UP_NO_: /* 電子票證加值 */
				case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_: /* 電子票證餘額查詢 */
				case _ECR_8N1_ESVC_VOID_TOP_UP_NO_: /* 電子票證加值取消 */
				case _ECR_8N1_HG_REWARD_SALE_NO_: /* HG紅利積點 */
				case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_: /* HG點數抵扣 */
				case _ECR_8N1_HG_POINT_CERTAIN_NO_: /* HG加價購 */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(szTemplate, &szDataBuffer[10], 1);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", szTemplate);
						inDISP_LogPrintf(szDebugMsg);
					}

					/* 欄位為M 卻不存在，回傳錯誤 */
					if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "Not Exist Error");
							inDISP_LogPrintf(szDebugMsg);
						}

						return (VS_ERROR);
					} else
					{
						memcpy(srECROb->srTransData.szCUPIndicator, szTemplate, 1);
					}

					break;
				case _ECR_8N1_PREAUTH_CANCEL_NO_: /* 預先授權取消 */
				case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_: /* 預先授權完成取消 */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(szTemplate, &szDataBuffer[10], 1);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "  CUPIndicator :%s  ", szTemplate);
						inDISP_LogPrintf(szDebugMsg);
					}

					/* 欄位為M 卻不存在，回傳錯誤 */
					if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "Not Exist Error");
							inDISP_LogPrintf(szDebugMsg);
						}

						return (VS_ERROR);
					} else
					{
						memcpy(srECROb->srTransData.szCUPIndicator, szTemplate, 1);
					}

					/* 預先授權取消及完成僅支援銀聯卡交易 ，故此二Request之 CUP Indicator欄位僅可放 ”C”*/
					if (memcmp(&srECROb->srTransData.szCUPIndicator[0], "C", 1) != 0)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "CUP Not\"C\" Error");
							inDISP_LogPrintf(szDebugMsg);
						}

						return (VS_ERROR);
					}
					break;
				default:
					break;
			}
		}

		/* 如果是'0'代表Hotkey選擇 */
		if (memcmp(&srECROb->srTransData.szCUPIndicator[0], "0", 1) == 0)
		{
			inRetVal = inRS232_ECR_SelectOtherCardType(pobTran);

			/* Select失敗 */
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
		}

		/* 如果是'C'代表是銀聯卡交易 */
		if (memcmp(&srECROb->srTransData.szCUPIndicator[0], "C", 1) == 0)
		{
			pobTran->srBRec.uszCUPTransBit = VS_TRUE;
		}

		/* 如果是'S'代表是SmartPay交易 */
		if (memcmp(&srECROb->srTransData.szCUPIndicator[0], "S", 1) == 0)
		{
			pobTran->srBRec.uszFiscTransBit = VS_TRUE;
			/* SmartPay不用簽名 */
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
		}

		/* 如果是'E'代表是電票交易 */
		if (memcmp(&srECROb->srTransData.szCUPIndicator[0], "E", 1) == 0)
		{
			pobTran->srTRec.uszESVCTransBit = VS_TRUE;
		}
	}

	/* 銀行別/付款別 HOST ID*/
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_: /* 取消交易 */
		case _ECR_8N1_TIP_NO_: /* 小費交易 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_: /* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_: /* 預先授權完成取消 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_: /* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_: /* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_: /* 總額查詢 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[11], 2);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			}

			memset(szHostLabel, 0x00, sizeof (szHostLabel));

			if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_));
			} else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_DCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
			}				/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_LOYALTY_REDEEM_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_));
			} else
			{
				return (VS_ERROR);
			}

			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					/* 如果return VS_ERROR 代表table沒有該HOST */
					inDISP_LogPrintfWithFlag(" ECR 8n1 Customer 107 Unpack Load HDT[%d] *Error* Line[%d]", i, __LINE__);
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof (szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof (szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inDISP_LogPrintf(szDebugMsg);
						}

						/* 如果return VS_ERROR 代表Host沒開 */
						return (VS_ERROR);
					} else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;

						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof (szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inDISP_LogPrintf(szDebugMsg);
						}

					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;

		default:
			break;
	}

	/* EDC簽單序號 Receipt No(InvoiceNumber) */
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_: /* 取消交易 */
		case _ECR_8N1_TIP_NO_: /* 小費交易 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_: /* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_: /* 預先授權完成取消 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_: /* 重印簽單 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(&szTemplate[0], &szDataBuffer[13], 6);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			} else
			{
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;
		default:
			break;
	}

	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* 啟動卡號查詢 */
		case _ECR_8N1_SALE_NO_: /* 一般交易 */
		case _ECR_8N1_REFUND_NO_: /* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_: /* 交易補登 */
		case _ECR_8N1_PREAUTH_NO_: /* 預先授權 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 */
		case _ECR_8N1_INSTALLMENT_NO_: /* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_: /* 紅利 */
		case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
		case _ECR_8N1_ESVC_TOP_UP_NO_: /* 電子票證加值 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_: /* 電子票證加值取消 */
		case _ECR_8N1_HG_REWARD_SALE_NO_: /* HG紅利積點 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_: /* 快樂購紅利積點 + 信用卡分期付款 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_: /* 快樂購紅利積點 + 信用卡紅利扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_: /* HG點數抵扣 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_: /* HG加價購 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(&szTemplate[0], &szDataBuffer[42], 10);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			} else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;
		default:
			break;
	}

	/* 交易日期 Trans Date */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* 啟動卡號查詢 */
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_: /* 結束卡號查詢*/
		case _ECR_8N1_SALE_NO_: /* 一般交易 */
		case _ECR_8N1_REFUND_NO_: /* 退貨交易 */
		case _ECR_8N1_VOID_NO_: /* 取消交易 */
		case _ECR_8N1_OFFLINE_NO_: /* 交易補登 */
		case _ECR_8N1_TIP_NO_: /* 小費交易 */
		case _ECR_8N1_PREAUTH_NO_: /* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_: /* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_: /* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_: /* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_: /* 紅利 */
		case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
		case _ECR_8N1_SETTLEMENT_NO_: /* 結帳交易 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_: /* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_: /* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_: /* 總額查詢 */
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_: /* 明細列印 */
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_: /* 總額列印 */
		case _ECR_8N1_ESVC_TOP_UP_NO_: /* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_: /* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_: /* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(&szTemplate[0], &szDataBuffer[54], 6);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Date", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				//				return (VS_ERROR);
			} else
			{
				sprintf(pobTran->srBRec.szDate, "20%s", szTemplate);
				sprintf(pobTran->srBRec.szOrgDate, "20%s", szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易日期", pobTran->srBRec.szDate);
				inDISP_LogPrintf(szDebugMsg);
			}
			break;
		default:
			break;
	}

	/* 交易時間 Trans Time */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* 啟動卡號查詢 */
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_: /* 結束卡號查詢*/
		case _ECR_8N1_SALE_NO_: /* 一般交易 */
		case _ECR_8N1_REFUND_NO_: /* 退貨交易 */
		case _ECR_8N1_VOID_NO_: /* 取消交易 */
		case _ECR_8N1_OFFLINE_NO_: /* 交易補登 */
		case _ECR_8N1_TIP_NO_: /* 小費交易 */
		case _ECR_8N1_PREAUTH_NO_: /* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_: /* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_: /* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_: /* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_: /* 紅利 */
		case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
		case _ECR_8N1_SETTLEMENT_NO_: /* 結帳交易 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_: /* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_: /* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_: /* 總額查詢 */
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_: /* 明細列印 */
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_: /* 總額列印 */
		case _ECR_8N1_ESVC_TOP_UP_NO_: /* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_: /* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_: /* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[60], 6);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Time", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				//				return (VS_ERROR);
			} else
			{
				sprintf(pobTran->srBRec.szTime, "%s", szTemplate);
				sprintf(pobTran->srBRec.szOrgTime, "%s", szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易時間", pobTran->srBRec.szTime);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;

		default:
			break;
	}

	/* 授權碼(Auth Code) ECR電文中給9個字元 但ATS電文只需要6個，所以只抓6個 */
	switch (inTransType)
	{
		case _ECR_8N1_REFUND_NO_: /* 退貨 */
		case _ECR_8N1_OFFLINE_NO_: /* 交易補登 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
		case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof (pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[66], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			break;
		case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 */
			/* 處理授權碼(must have) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof (pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[66], 6);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Code", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			} else
			{
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}

			break;

		default:
			break;
	}

	/* 其他金額(Exp Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_TIP_NO_: /* 小費交易 */
			/* 小費金額*/
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memset(szTemplate, 0x00, sizeof (szTemplate));
			inGetTRTFileName(szTemplate);
			if (!memcmp(szTemplate, "DCCTRT", 6) && !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
			{
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[103], 12);
			} else
			{
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[103], 10);
			}

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Exp Amount", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			}

			pobTran->srBRec.lnTipTxnAmount = atol(szTemplate);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tip: %ld", pobTran->srBRec.lnTipTxnAmount);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;

		case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 預先授權完成交易之原預先授權金額 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[103], 10);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Exp Amount", "Not Exist Error");
					inDISP_LogPrintf(szDebugMsg);
				}

				return (VS_ERROR);
			}

			pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
			break;

		default:
			break;
	}

	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* 啟動卡號查詢 */
		case _ECR_8N1_SALE_NO_: /* 一般交易 */
		case _ECR_8N1_REFUND_NO_: /* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_: /* 交易補登 */
		case _ECR_8N1_PREAUTH_NO_: /* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_: /* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_: /* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_: /* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_: /* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_: /* 紅利 */
		case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
		case _ECR_8N1_HG_REWARD_SALE_NO_: /* HG紅利積點 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_: /* 快樂購紅利積點 + 信用卡分期付款 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_: /* 快樂購紅利積點 + 信用卡紅利扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_: /* 快樂購點數扣抵 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_: /* 快樂購加價購 */
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_: /* 快樂購點數兌換 */
		case _ECR_8N1_HG_REDEEM_REFUND_NO_: /* 快樂購扣抵退貨 */
		case _ECR_8N1_HG_REWARD_REFUND_NO_: /* 快樂購回饋退貨 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[115], 18);

			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_: /* 電子票證加值 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_: /* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			memcpy(szTemplate, &szDataBuffer[115], 18);

			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srTRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srTRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srTRec.szStoreID);
				inDISP_LogPrintf(szDebugMsg);
			}

			break;
		default:
			break;
	}

	if (inTransType == _ECR_8N1_AWARD_REDEEM_NO_)
	{

	} else if (inTransType == _ECR_8N1_VOID_AWARD_REDEEM_NO_)
	{

	} else
	{
		/* 實際支付金額RDM Paid Amt (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
			case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_: /* 快樂購紅利積點 + 信用卡紅利扣抵 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[134], 10); /* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "RDM Paid Amt", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 信用卡紅利扣抵點數 RDM Point */
		switch (inTransType)
		{
			case _ECR_8N1_REDEEM_REFUND_NO_: /* 紅利退貨 */
			case _ECR_8N1_REDEEM_ADJUST_NO_: /* 紅利調帳 */
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_: /* 快樂購紅利積點 + 信用卡紅利扣抵 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[146], 8);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "RDM Point", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 分期期數 Installment Period */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_NO_: /* 分期 */
			case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_: /* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[174], 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Installment Period", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 首期金額 Down Payment Amount (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_: /* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[176], 10); /* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Down Payment Amount", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 每期金額 Installment Payment Amount (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_: /* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[188], 10); /* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Installment Payment Amount", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 分期手續費 Formallity Fee (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_: /* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_: /* 分期調帳 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[200], 10); /* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}
				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

				if (pobTran->srBRec.lnInstallmentFormalityFee != 0)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_;

					return (VS_ERROR);
				}

				break;
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_: /* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				memcpy(szTemplate, &szDataBuffer[200], 10); /* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
					inDISP_LogPrintf(szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 啟動卡號查詢 Start Trans Type */
		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* 卡號查詢 */

				break;
			default:
				break;
		}

		/* 只有金融卡才要收的欄位 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/* 金融卡原交易日期 SP Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_: /* 退費 */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(szTemplate, &szDataBuffer[231], 8);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "SP Origin Date", szTemplate);
						inDISP_LogPrintf(szDebugMsg);
					}

					memcpy(pobTran->srBRec.szFiscRefundDate, szTemplate, 8);
					break;
				default:
					break;
			}

			/* 金融卡調單編號 SP RRN */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_: /* 退費 */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(szTemplate, &szDataBuffer[239], 12);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "SP RRN", szTemplate);
						inDISP_LogPrintf(szDebugMsg);
					}

					memcpy(pobTran->srBRec.szFiscRRN, szTemplate, 12);

					break;
				default:
					break;
			}

		}			/* 只有電票要收的欄位 */
		else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* ATS電票交易序號 RF NUMBER 左靠右補空白 */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_: /* 電票退貨 */
					memset(szTemplate, 0x00, sizeof (szTemplate));
					memcpy(szTemplate, &szDataBuffer[239], 12);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "RF NUMBER", szTemplate);
						inDISP_LogPrintf(szDebugMsg);
					}

					memcpy(pobTran->srTRec.szTicketRefundCode, szTemplate, 12);

					break;
				default:
					break;
			}
		}

	}
	/* 因為要修改掉NCCC的規則,所以拿掉 20190328 [SAM] */
#if 0		
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* 有開CUP且MACEnable有開但安全認證沒過，不能執行CUP交易 */
		if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
		{
			if (inNCCC_Func_CUP_PowerOn_LogOn(pobTran) != VS_SUCCESS)
			{
				/* 安全認證失敗 */
				return (VS_ERROR);
			}
		}
	}
#endif	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_8N1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
				pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;

				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜銀聯一般交易＞ */

				pobTran->inFunctionID = _CUP_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = _CUP_SALE_;
				pobTran->srBRec.inOrgCode = _CUP_SALE_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
				} else
				{
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
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

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}

			break;
		case _ECR_8N1_VOID_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜沖正交易＞ */

				pobTran->inFunctionID = _FISC_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_FISC_VOID_;

				pobTran->inTransactionCode = _FISC_VOID_;
				pobTran->srBRec.inCode = _FISC_VOID_;
				pobTran->srBRec.inOrgCode = _FISC_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = _CUP_VOID_;
				pobTran->srBRec.inOrgCode = _CUP_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_8N1_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _CUP_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
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

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜退貨交易＞ */

				pobTran->inFunctionID = _TICKET_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_REFUND_;
				} else
				{
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_REFUND_;
				pobTran->srTRec.inCode = _TICKET_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
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

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_8N1_OFFLINE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜交易補登＞ */

			pobTran->inFunctionID = _SALE_OFFLINE_;
			pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

			pobTran->inTransactionCode = _SALE_OFFLINE_;
			pobTran->srBRec.inCode = _SALE_OFFLINE_;
			pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_PREAUTH_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權＞ */

				pobTran->inFunctionID = _PRE_AUTH_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inTransactionCode = _PRE_AUTH_;
				pobTran->srBRec.inCode = _PRE_AUTH_;
				pobTran->srBRec.inOrgCode = _PRE_AUTH_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權＞ */

				pobTran->inFunctionID = _PRE_AUTH_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inTransactionCode = _PRE_AUTH_;
				pobTran->srBRec.inCode = _PRE_AUTH_;
				pobTran->srBRec.inOrgCode = _PRE_AUTH_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}

			break;

		case _ECR_8N1_PREAUTH_CANCEL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_AUTH_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權取消＞ */

			pobTran->inFunctionID = _CUP_PRE_AUTH_VOID_;
			pobTran->inRunOperationID = _OPERATION_VOID_;
			pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_VOID_;

			pobTran->inTransactionCode = _CUP_PRE_AUTH_VOID_;
			pobTran->srBRec.inCode = _CUP_PRE_AUTH_VOID_;
			pobTran->srBRec.inOrgCode = _CUP_PRE_AUTH_VOID_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_PREAUTH_COMPLETE_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權完成＞ */

				pobTran->inFunctionID = _CUP_PRE_COMP_;
				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_;
				}
				pobTran->inRunTRTID = _TRT_PRE_COMP_;

				pobTran->inTransactionCode = _CUP_PRE_COMP_;
				pobTran->srBRec.inCode = _CUP_PRE_COMP_;
				pobTran->srBRec.inOrgCode = _CUP_PRE_COMP_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權完成＞ */

				pobTran->inFunctionID = _PRE_COMP_;
				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_;
				}
				pobTran->inRunTRTID = _TRT_PRE_COMP_;

				pobTran->inTransactionCode = _PRE_COMP_;
				pobTran->srBRec.inCode = _PRE_COMP_;
				pobTran->srBRec.inOrgCode = _CUP_PRE_COMP_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_COMP_VOID_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜預先授權完成取消＞ */

			pobTran->inFunctionID = _CUP_PRE_COMP_VOID_;
			pobTran->inRunOperationID = _OPERATION_VOID_;
			pobTran->inRunTRTID = _TRT_CUP_PRE_COMP_VOID_;

			pobTran->inTransactionCode = _CUP_PRE_COMP_VOID_;
			pobTran->srBRec.inCode = _CUP_PRE_COMP_VOID_;
			pobTran->srBRec.inOrgCode = _CUP_PRE_COMP_VOID_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_INSTALLMENT_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 保留給銀聯分期使用 */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 保留給銀聯分期退貨使用 */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜分期調帳＞ */

			pobTran->inFunctionID = _INST_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_INST_ADJUST_;
			pobTran->inRunTRTID = _TRT_INST_ADJUST_;

			pobTran->inTransactionCode = _INST_ADJUST_;
			pobTran->srBRec.inCode = _INST_ADJUST_;
			pobTran->srBRec.inOrgCode = _INST_ADJUST_;

			pobTran->srBRec.uszInstallmentBit = VS_TRUE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_REDEEM_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 保留給銀聯紅利扣抵使用 */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_8N1_REDEEM_REFUND_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 保留給銀聯紅利退貨使用 */
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_CUP_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			} else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;

				memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
				inGetContactlessEnable(szCTLSEnable);
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_;
				} else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;

		case _ECR_8N1_REDEEM_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜紅利調帳＞ */

			pobTran->inFunctionID = _REDEEM_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_REDEEM_ADJUST_;
			pobTran->inRunTRTID = _TRT_REDEEM_ADJUST_;

			pobTran->inTransactionCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inOrgCode = _REDEEM_ADJUST_;

			pobTran->srBRec.uszRedeemBit = VS_TRUE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_TIP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TIP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜小費交易＞ */

			pobTran->inFunctionID = _TIP_;
			pobTran->inRunOperationID = _OPERATION_TIP_;
			pobTran->inRunTRTID = _TRT_TIP_;

			pobTran->inTransactionCode = _TIP_;
			pobTran->srBRec.inCode = _TIP_;
			pobTran->srBRec.inOrgCode = _TIP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_AWARD_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜優惠兌換＞ */

			pobTran->inFunctionID = _LOYALTY_REDEEM_;
			pobTran->inRunOperationID = _OPERATION_LOYALTY_REDEEM_CTLS_;
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;

			pobTran->inTransactionCode = _LOYALTY_REDEEM_;
			pobTran->srBRec.inCode = _LOYALTY_REDEEM_;
			pobTran->srBRec.inOrgCode = _LOYALTY_REDEEM_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_VOID_AWARD_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_VOID_LOYALTY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜兌換取消＞ */

			pobTran->inFunctionID = _VOID_LOYALTY_REDEEM_;
			pobTran->inRunOperationID = _OPERATION_VOID_LOYALTY_REDEEM_;
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;

			pobTran->inTransactionCode = _VOID_LOYALTY_REDEEM_;
			pobTran->srBRec.inCode = _VOID_LOYALTY_REDEEM_;
			pobTran->srBRec.inOrgCode = _VOID_LOYALTY_REDEEM_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;


			/* 快樂購紅利積點 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡紅利積點＞ */

			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_CREDIT_CTLS_;

			/* 這邊先設成sale，若之後支付方式選銀聯卡，會在那邊改成CUP_SALE */
			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購紅利積點─分期付款 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡分期付款＞ */

			pobTran->inFunctionID = _INST_SALE_;
			pobTran->inRunOperationID = _OPERATION_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_INSTALLMENT_CTLS_;

			pobTran->inTransactionCode = _INST_SALE_;
			pobTran->srBRec.inCode = _INST_SALE_;
			pobTran->srBRec.inOrgCode = _INST_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購紅利積點─紅利扣抵 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡紅利扣抵＞ */

			pobTran->inFunctionID = _REDEEM_SALE_;
			pobTran->inRunOperationID = _OPERATION_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REDEMPTION_CTLS_;

			pobTran->inTransactionCode = _REDEEM_SALE_;
			pobTran->srBRec.inCode = _REDEEM_SALE_;
			pobTran->srBRec.inOrgCode = _REDEEM_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszRedeemBit = VS_TRUE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購點數扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡點數扣抵＞ */

			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_ONLINE_REDEEM_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_ONLINE_REDEEM_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購加價購 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡加價購＞ */

			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_POINT_CERTAIN_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_POINT_CERTAIN_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購點數兌換 */
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡點數兌換＞ */

			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_FULL_REDEMPTION_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_FULL_REDEMPTION_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購扣抵退貨 */
		case _ECR_8N1_HG_REDEEM_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡扣抵退貨＞ */

			pobTran->inFunctionID = _HG_REDEEM_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REDEEM_REFUND_;

			pobTran->inTransactionCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REDEEM_REFUND_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購回饋退貨 */
		case _ECR_8N1_HG_REWARD_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡回饋退貨＞ */

			pobTran->inFunctionID = _HG_REWARD_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REFUND_;

			pobTran->inTransactionCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_REFUND_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

			/* 快樂購點數查詢 */
		case _ECR_8N1_HG_POINT_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜HG卡點數查詢＞ */

			pobTran->inFunctionID = _HG_INQUIRY_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_INQUIRY_;

			pobTran->inTransactionCode = _HG_INQUIRY_;
			pobTran->srBRec.inCode = _HG_INQUIRY_;
			pobTran->srBRec.inOrgCode = _HG_INQUIRY_;
			pobTran->srBRec.lnHGTransactionType = _HG_INQUIRY_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;

			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_REPRINT_RECEIPT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜重印帳單＞ */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REPRINT_;
			pobTran->inRunTRTID = FALSE;

			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 <交易查詢> */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_DETAIL_;
			pobTran->inRunTRTID = FALSE;

			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜總額查詢＞ */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_TOTAL_;
			pobTran->inRunTRTID = FALSE;

			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 明細列印 */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_DETAIL_REPORT_;
			pobTran->inRunTRTID = FALSE;

			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 總額列印 */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_TOTAL_REPORT_;
			pobTran->inRunTRTID = FALSE;

			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_EDC_REBOOT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜重新開機＞ */
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REBOOT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜重新開機＞ */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunTRTID = FALSE;

			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;

			/* 直接用function 不跑OPT */
			inFunc_Reboot();

			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜加值交易＞ */

			pobTran->inFunctionID = _TICKET_TOP_UP_;

			memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
			inGetContactlessEnable(szCTLSEnable);
			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_TOP_UP_;
			} else
			{
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_TOP_UP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜餘額查詢＞ */

			pobTran->inFunctionID = _TICKET_INQUIRY_;

			memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
			inGetContactlessEnable(szCTLSEnable);
			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_INQUIRY_;
			} else
			{
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_INQUIRY_;
			pobTran->srTRec.inCode = _TICKET_INQUIRY_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜加值取消＞ */

			pobTran->inFunctionID = _TICKET_VOID_TOP_UP_;

			memset(szCTLSEnable, 0x00, sizeof (szCTLSEnable));
			inGetContactlessEnable(szCTLSEnable);
			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_VOID_TOP_UP_;
			} else
			{
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_VOID_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_VOID_TOP_UP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_;
			inRetVal = VS_ERROR;
			break;
			/* 客製化107直接借用來當Echo Test*/
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_: /* Echo Test */
			inRetVal = VS_SUCCESS;
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}

	return (inRetVal);
}

/*
Function        :inRS232_ECR_8N1_Customer_107_Bumper_Pack
Date&Time       :2016/7/6 下午 2:40
Describe        :先把要送的資料組好
 */
int inECR_8N1_Customer_107_Bumper_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int inRetVal = VS_ERROR;
	int i = 0, inCardLen = 0;
	int inPacketSizes = 0;
	char szTemplate[100 + 1];
	char szHash[44 + 1];
	char szFESMode[2 + 1];
	ACCUM_TOTAL_REC srAccumRec;

	memset(&srAccumRec, 0x00, sizeof (srAccumRec));

	/* ECR Indicator (1 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "I", 1);
	inPacketSizes++;
	/* ECR Version Date (6 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "160407", 6);
	inPacketSizes += 6;
	/* Trans Type Indicator (1 Byte) */
	inPacketSizes++;
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;
	/* CUP Indicator (1 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2))
	{
		if (!memcmp(srECROb->srTransData.szCUPIndicator, "C", 1) || pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "C", 1);
		else if (!memcmp(srECROb->srTransData.szCUPIndicator, "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "S", 1);
		else if (!memcmp(srECROb->srTransData.szCUPIndicator, "E", 1) || pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "E", 1);
		else
			memcpy(&szDataBuffer[inPacketSizes], "N", 1);
	}

	inPacketSizes++;

	/* HOST ID (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetTRTFileName(szTemplate);

		if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_NCCC_, 2);
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_HG_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_DCC_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_ESVC_, 2);
	}		/* 客製化107預計只回傳03 */
		/* 先在客製化107上實做信用卡比數回傳 */
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2))
	{
		memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_NCCC_, 2);
	}

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			sprintf(szTemplate, "%06ld", pobTran->srTRec.lnInvNum);
		} else
		{
			sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		}
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Card No (19 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2) ||
			pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
	{
		/* 優惠兌換先設定不回傳卡號 */
		inPacketSizes += 19;
	} else
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTRec.szUID[0], strlen(pobTran->srTRec.szUID));
		} else
		{
			/* 卡號是否遮掩 */
			inRetVal = inECR_CardNoTruncateDecision(pobTran);
			/* 要遮卡號 */
			if (inRetVal == VS_SUCCESS)
			{
				memset(szTemplate, 0x00, sizeof (szTemplate));
				strcpy(szTemplate, pobTran->srBRec.szPAN);

				/* HAPPG_GO 卡不掩飾 */
				if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
				{

				} else
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
						for (i = 2; i < (inCardLen - 4); i++)
							szTemplate[i] = '*';
					} else
					{
						for (i = 6; i < (inCardLen - 4); i++)
							szTemplate[i] = '*';
					}

				}

				memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
			} else
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
			}
		}
		inPacketSizes += 19;
	}

	/* Card Expire Date (4 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2))
	{
		inPacketSizes += 4;
	} else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		inPacketSizes += 4;
	}		/* 先在客製化107上實做信用卡比數回傳 */
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2))
	{
		pobTran->srBRec.inHDTIndex = 0;
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" ECR Customer 107 Pack Load HDT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			inDISP_DispLogAndWriteFlie(" ECR Customer 107 Pack Load HDPT[%d] *Error* Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
		}

		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%04ld", srAccumRec.lnTotalCount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		} else if (inRetVal == VS_NO_RECORD)
		{
			memcpy(&szDataBuffer[inPacketSizes], "0000", strlen("0000"));
		} else
		{
			memcpy(&szDataBuffer[inPacketSizes], "0000", strlen("0000"));
		}

		inPacketSizes += 4;
	} else
	{
		/* 有效期是否回傳 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetECRExpDateReturnEnable(szTemplate);

		/* 要回傳有效期 */
		if (!memcmp(&szTemplate, "Y", 1))
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

		inPacketSizes += 4;
	}

	/* Trans Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2))
	{
		inPacketSizes += 12;
	}		/* 先在客製化107上實做信用卡比數回傳 */
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2))
	{
		pobTran->srBRec.inHDTIndex = 0;
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%010lld00", srAccumRec.llTotalAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		} else if (inRetVal == VS_NO_RECORD)
		{
			memcpy(&szDataBuffer[inPacketSizes], "000000000000", strlen("000000000000"));
		} else
		{
			memcpy(&szDataBuffer[inPacketSizes], "000000000000", strlen("000000000000"));
		}

		inPacketSizes += 12;
	} else if (pobTran->srTRec.lnTxnAmount != 0)
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srTRec.lnTxnAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

		inPacketSizes += 12;
	} else if (pobTran->srBRec.lnTxnAmount != 0)
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

		inPacketSizes += 12;
	} else
	{
		inPacketSizes += 12;
	}

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2))
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* Trans Date */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			strcat(szTemplate, pobTran->srTRec.szDate);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;

			/* Trans Time */
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srTRec.szTime, 6);
			inPacketSizes += 6;
		} else
		{
			/* Trans Date */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			strcat(szTemplate, pobTran->srBRec.szDate);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
			inPacketSizes += 6;

			/* Trans Time */
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
			inPacketSizes += 6;
		}
	} else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			strcpy(szTemplate, pobTran->srTRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PAD_LEFT_FILL_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		} else
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PAD_LEFT_FILL_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
	}

	inPacketSizes += 9;

	/* Wave Card Indicator (1 Byte) */
	/* Add by hanlin 2012/10/16 AM 11:01 修改僅一般交易跟卡號查詢會回傳 Wave Card Indicator */
	if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
#if 0 //先刪除		
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
			memcpy(&szDataBuffer[inPacketSizes], "P", 1);
		} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
			memcpy(&szDataBuffer[inPacketSizes], "Z", 1);
		} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
		{
			memcpy(&szDataBuffer[inPacketSizes], "G", 1);
		} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_HAPPYCASH_)
		{
			memcpy(&szDataBuffer[inPacketSizes], "H", 1);
		} else
#endif					
		{
			memcpy(&szDataBuffer[inPacketSizes], "O", 1);
		}
	} else
	{
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2))
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			{
				/* 規格未寫M 跟單機同步 */
				memset(szTemplate, 0x00, sizeof (szTemplate));
				inGetCardLabel(szTemplate);

				if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
					memcpy(&szDataBuffer[inPacketSizes], "V", 1);
				else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
					memcpy(&szDataBuffer[inPacketSizes], "M", 1);
				else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
					memcpy(&szDataBuffer[inPacketSizes], "J", 1);
				else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
					memcpy(&szDataBuffer[inPacketSizes], "C", 1);
				else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
					memcpy(&szDataBuffer[inPacketSizes], "A", 1);
				else
					memcpy(&szDataBuffer[inPacketSizes], "O", 1);
			}
		}
	}
	inPacketSizes++;

	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);

	inPacketSizes += 4;

	/* Merchant ID (15 Byte) & Terminal ID (8 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2))
	{
		inPacketSizes += 15;
		inPacketSizes += 8;
	} else
	{
		/* Merchant ID (15 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetMerchantID(szTemplate);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PAD_LEFT_FILL_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 15);
		inPacketSizes += 15;
		/* Terminal ID (8 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetTerminalID(szTemplate);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PAD_LEFT_FILL_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
		inPacketSizes += 8;
	}

	/* Exp Amount (12 Byte) */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	inGetTRTFileName(szTemplate);
	if (!memcmp(szTemplate, "DCCTRT", 6) && !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
	{
		if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "0", 1))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%010ld00", atol(pobTran->srBRec.szDCC_TIPFCA));
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		} else if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "1", 1))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%011ld0", atol(pobTran->srBRec.szDCC_TIPFCA));
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		} else if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "2", 1))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%012ld", atol(pobTran->srBRec.szDCC_TIPFCA));
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		}
	} else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTipTxnAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}

	inPacketSizes += 12;

	/* Store Id (18 Byte) */
	inPacketSizes += 18;

	/* 處理紅利、分期 */
	/* Installment / Redeem Indictor (1 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2))
	{
		if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szInstallmentIndicator[0], 1);
		else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szRedeemIndicator[0], 1);
	}

	/* 優惠兌換方式 */
	/* 1.條碼兌換 2.卡號兌換 */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
	{
		if (!memcmp(pobTran->szL3_AwardWay, "1", 1) ||
				!memcmp(pobTran->szL3_AwardWay, "2", 1) ||
				!memcmp(pobTran->szL3_AwardWay, "3", 1))
		{
			memcpy(&szDataBuffer[inPacketSizes], "1", 1);
		} else if (!memcmp(pobTran->szL3_AwardWay, "4", 1) ||
				!memcmp(pobTran->szL3_AwardWay, "5", 1))
		{
			memcpy(&szDataBuffer[inPacketSizes], "2", 1);
		}
	} else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2))
	{
		/* (1= 條碼兌換， 2= 卡號兌換 )*/
		/* 取消優惠兌換只接受 條碼兌換 。 */
		memcpy(&szDataBuffer[inPacketSizes], "1", 1);
	}
	inPacketSizes++;

	/* 處理紅利扣抵 */
	if ((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)) && pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 支付金額 RDM Paid Amt (12 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
		/* 紅利扣抵點數 RDM Point (8 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%08lu", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
		inPacketSizes += 8;
		/* 紅利剩餘點數 Points Of Balance (8 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%08lu", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
		inPacketSizes += 8;
		/* 紅利扣抵金額 Redeem Amt (12 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
	} else
		inPacketSizes += 40;

	/* 處理分期交易 */
	if ((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)) && pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 分期期數 Installment Period (2 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);
		inPacketSizes += 2;
		/* 首期金額 Down Payment (12 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
		/* 每期金額 Installment Payment Amt (12 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
		/* 分期手續費 Formlity Fee (12 Byte) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentFormalityFee);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
	} else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		inPacketSizes += 2;

		/* 交易前餘額 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
		{
			if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
				sprintf(szTemplate, "-%09lu00", (unsigned long) (pobTran->srTRec.lnTotalTopUpAmount + (pobTran->srTRec.lnFinalBeforeAmt - 100000)));
			else
				sprintf(szTemplate, "+%09lu00", (unsigned long) (pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTotalTopUpAmount));
		} else
		{
			if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
				sprintf(szTemplate, "-%09lu00", (unsigned long) (pobTran->srTRec.lnFinalBeforeAmt - 100000));
			else
				sprintf(szTemplate, "+%09lu00", (unsigned long) (pobTran->srTRec.lnFinalBeforeAmt));
		}

		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		inPacketSizes += 12;

		/* 交易後餘額 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		if (pobTran->srTRec.lnFinalAfterAmt > 100000)
			sprintf(szTemplate, "-%09lu00", (unsigned long) (pobTran->srTRec.lnFinalAfterAmt - 100000));
		else
			sprintf(szTemplate, "+%09lu00", (unsigned long) (pobTran->srTRec.lnFinalAfterAmt));

		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		inPacketSizes += 12;

		/* 自動加值金額 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
				sprintf(szTemplate, "+%09lu00", (unsigned long) (pobTran->srTRec.lnTotalTopUpAmount));
			else
				sprintf(szTemplate, "+%09lu00", (unsigned long) (0));

			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		}
		inPacketSizes += 12;
	} else
		inPacketSizes += 38;

	/* Card Type (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) && pobTran->srBRec.uszHappyGoSingle != VS_TRUE) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)) && pobTran->srBRec.uszHappyGoMulti == VS_TRUE))
	{

		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
#if 0 //先刪除					
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_IPASS_, 2);
			} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_ECC_, 2);
			} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_ICASH_, 2);
			} else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_HAPPYCASH_)
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_HAPPYCASH_, 2);
			} else
#endif 				
				memcpy(&szDataBuffer[inPacketSizes], "FF", 2);
		} else
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			inGetCardLabel(szTemplate);

			if (!memcmp(szTemplate, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_UCARD_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_VISA_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_MASTERCARD_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_JCB_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_AMEX_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_CUP_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_DINERS_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_SMARTPAY_, strlen(_CARD_TYPE_SMARTPAY_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_SMARTPAY_, 2);
			else
				memcpy(&szDataBuffer[inPacketSizes], "FF", 2);
		}
	}

	inPacketSizes += 2;

	/* Batch No (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		inGetBatchNum(szTemplate);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Start Trans Type (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2))
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szStartTransType, 2);

	inPacketSizes += 2;

	/* MP Flag(是否小額交易，MPAS小額交易用) (1 Byte) */
	/* 合併MPAS ECR到標準400 */
	memset(szFESMode, 0x00, sizeof (szFESMode));
	inGetNCCCFESMode(szFESMode);
	if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
	{
		if (pobTran->srBRec.uszMPASTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "M", 1);
		else
			memcpy(&szDataBuffer[inPacketSizes], " ", 1);
	}
	inPacketSizes += 1;


	memset(szTemplate, 0x00, sizeof (szTemplate));
	inGetPayItemEnable(szTemplate);
	if (memcmp(szTemplate, "Y", strlen("Y")) == 0 &&
			(strlen(pobTran->srBRec.szPayItemCode) > 0))
	{
		/* SmartPay要回傳這三項資訊 (99 Bytes) */
		if (!memcmp(&srECROb->srTransData.szCUPIndicator[0], "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/* SP ISSUER ID (8 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscIssuerID[0], 8);
			inPacketSizes += 8;
			/* SP Origin Date (8 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscRefundDate[0], 8);
			inPacketSizes += 8;
			/* SP RRN (12 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscRRN[0], 12);
			inPacketSizes += 12;
		} else
		{
			inPacketSizes += 8;
			inPacketSizes += 8;
			inPacketSizes += 12;
		}

		/* Pay Item (5 Bytes) */
		/* ECR一段式收銀機連線 送空白payitem 不必回傳 */
		if (srECROb->srTransData.uszECRResponsePayitem == VS_FALSE)
		{
			inPacketSizes += 5;
		} else
		{
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szPayItemCode[0], 5);
			inPacketSizes += 5;
		}

		/* 【需求單 - 105039】信用卡為電子發票載具，端末機將卡號加密後回傳收銀機 add by LingHsiung 2016-04-20 上午 09:56:24 */
		/*
			步驟1. Hash Data = 卡號 (U Card則取11碼卡號進行HASH)
			步驟2. Hash Value(32Bytes) = 以SHA-256演算Hash Data
			步驟3. BASE64 Data(44 Bytes) = 將Hash Value以BASE64進行編碼
			步驟4. 加密卡號(50 Bytes) = BIN N0(6 Bytes) + BASE64 Data(44 Bytes)。
			步驟5. 將加密卡號(50 Bytes)依電文規格指定欄位(37)回傳收銀機。
		 */
		/* Card No. Hash Value (50 Bytes) */
		memset(szTemplate, 0x00, sizeof (szTemplate));

		if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)))
			memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[4], 11);
		else
			memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));

		if (strlen(szTemplate) > 0)
		{
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;
			memset(szHash, 0x00, sizeof (szHash));
			inFunc_CardNumber_Hash(szTemplate, szHash);
			memcpy(&szDataBuffer[inPacketSizes], &szHash[0], 44);
			inPacketSizes += 44;
		} else
		{
			inPacketSizes += 50;
		}

		/* MP Response Code (6 Bytes) */
		memset(szFESMode, 0x00, sizeof (szFESMode));
		inGetNCCCFESMode(szFESMode);
		if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
		{
			if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0 &&
					memcmp(pobTran->srBRec.szRespCode, "11", 2) != 0 &&
					pobTran->srBRec.uszMPASTransBit == VS_TRUE &&
					(memcmp(&pobTran->srBRec.szMPASAuthCode[0], "CAF", 3) == 0 ||
					memcmp(&pobTran->srBRec.szMPASAuthCode[0], "NEG", 3) == 0 ||
					memcmp(&pobTran->srBRec.szMPASAuthCode[0], "WEB", 3) == 0 ||
					memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJ", 3) == 0))
			{
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMPASAuthCode[0], 6);
			}
		}
		inPacketSizes += 6;

		/* ASM award flag */
		inPacketSizes += 1;

		/* 【需求單 - 106128】行動支付標記及金融機構代碼 add by LingHsiung 2017-08-23 上午 11:48:57 */
		if (strlen(pobTran->srBRec.szMCP_BANKID) > 0)
		{
			/* MCP Indicator (1 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[0], 1);
			inPacketSizes += 1;

			/* 金融機構代碼 (3 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[1], 3);
			inPacketSizes += 3;
		}

		/* Reserve (5 Bytes) */
		inPacketSizes += 5;
	} else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		inPacketSizes += 8;
		inPacketSizes += 8;

		/* RF序號 */
		memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTRec.szTicketRefundCode[0], 6);
		inPacketSizes += 12;

		inPacketSizes += 71;
	} else
	{
		/* SmartPay要回傳這三項資訊 (99 Bytes) */
		if (!memcmp(&srECROb->srTransData.szCUPIndicator[0], "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/* SP ISSUER ID (8 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscIssuerID[0], 8);
			inPacketSizes += 8;
			/* SP Origin Date (8 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscRefundDate[0], 8);
			inPacketSizes += 8;
			/* SP RRN (12 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscRRN[0], 12);
			inPacketSizes += 12;
			/* Payitem (5 Byte) */
			inPacketSizes += 5;
			/* 【需求單 - 105039】信用卡為電子發票載具，端末機將卡號加密後回傳收銀機 add by LingHsiung 2016-04-20 上午 09:56:24 */
			/* Card No. Hash Value (50 Bytes) */
			memset(szTemplate, 0x00, sizeof (szTemplate));

			if (memcmp(&pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
				memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[4], 11);
			else
				memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));


			if (strlen(szTemplate) > 0)
			{
				memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
				inPacketSizes += 6;

				memset(szHash, 0x00, sizeof (szHash));
				inFunc_CardNumber_Hash(szTemplate, szHash);
				memcpy(&szDataBuffer[inPacketSizes], &szHash[0], 44);
				inPacketSizes += 44;
			} else
			{
				inPacketSizes += 50;
			}

			/* MP Response Code (6 Byte) */
			memset(szFESMode, 0x00, sizeof (szFESMode));
			inGetNCCCFESMode(szFESMode);
			if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
			{
				if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0 &&
						memcmp(pobTran->srBRec.szRespCode, "11", 2) != 0 &&
						pobTran->srBRec.uszMPASTransBit == VS_TRUE &&
						(memcmp(&pobTran->srBRec.szMPASAuthCode[0], "CAF", 3) == 0 ||
						memcmp(&pobTran->srBRec.szMPASAuthCode[0], "NEG", 3) == 0 ||
						memcmp(&pobTran->srBRec.szMPASAuthCode[0], "WEB", 3) == 0 ||
						memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJ", 3) == 0))
				{
					memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMPASAuthCode[0], 6);
				}
			}
			inPacketSizes += 6;

			/* ASM award flag */
			inPacketSizes += 1;

			/* 【需求單 - 106128】行動支付標記及金融機構代碼 */
			if (strlen(pobTran->srBRec.szMCP_BANKID) > 0)
			{
				/* MCP Indicator (1 Bytes) */
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[0], 1);
				inPacketSizes += 1;

				/* 金融機構代碼 (3 Bytes) */
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[1], 3);
				inPacketSizes += 3;
			} else
			{
				inPacketSizes += 4;
			}

			/* Reserve (5 Bytes) */
			inPacketSizes += 5;
		} else
		{
			/* SP ISSUER ID (8 Byte) */
			inPacketSizes += 8;
			/* SP Origin Date (8 Byte) */
			inPacketSizes += 8;
			/* SP RRN (12 Byte) */
			inPacketSizes += 12;
			/* Payitem (5 Byte) */
			inPacketSizes += 5;
			/* 【需求單 - 105039】信用卡為電子發票載具，端末機將卡號加密後回傳收銀機 add by LingHsiung 2016-04-20 上午 09:56:24 */
			/* Card No. Hash Value (50 Bytes) */
			memset(szTemplate, 0x00, sizeof (szTemplate));

			if (memcmp(&pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
				memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[4], 11);
			else
				memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));


			if (strlen(szTemplate) > 0)
			{
				memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
				inPacketSizes += 6;

				memset(szHash, 0x00, sizeof (szHash));
				inFunc_CardNumber_Hash(szTemplate, szHash);
				memcpy(&szDataBuffer[inPacketSizes], &szHash[0], 44);
				inPacketSizes += 44;
			} else
			{
				inPacketSizes += 50;
			}

			/* MP Response Code (6 Byte) */
			memset(szFESMode, 0x00, sizeof (szFESMode));
			inGetNCCCFESMode(szFESMode);
			if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
			{
				if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0 &&
						memcmp(pobTran->srBRec.szRespCode, "11", 2) != 0 &&
						pobTran->srBRec.uszMPASTransBit == VS_TRUE &&
						(memcmp(&pobTran->srBRec.szMPASAuthCode[0], "CAF", 3) == 0 ||
						memcmp(&pobTran->srBRec.szMPASAuthCode[0], "NEG", 3) == 0 ||
						memcmp(&pobTran->srBRec.szMPASAuthCode[0], "WEB", 3) == 0 ||
						memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJ", 3) == 0))
				{
					memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMPASAuthCode[0], 6);
				}
			}
			inPacketSizes += 6;

			/* ASM award flag */
			if ((pobTran->srBRec.uszRewardL1Bit == VS_TRUE ||
					pobTran->srBRec.uszRewardL2Bit == VS_TRUE ||
					pobTran->srBRec.uszRewardL5Bit == VS_TRUE) &&
					pobTran->srBRec.uszRewardSuspendBit != VS_TRUE)
			{
				memcpy(&szDataBuffer[inPacketSizes], "A", strlen("A"));
			} else
			{
				memcpy(&szDataBuffer[inPacketSizes], " ", strlen(" "));
			}
			inPacketSizes += 1;

			/* 【需求單 - 106128】行動支付標記及金融機構代碼 */
			if (strlen(pobTran->srBRec.szMCP_BANKID) > 0)
			{
				/* MCP Indicator (1 Bytes) */
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[0], 1);
				inPacketSizes += 1;

				/* 金融機構代碼 (3 Bytes) */
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[1], 3);
				inPacketSizes += 3;
			} else
			{
				inPacketSizes += 4;
			}

			/* Reserve (5 Bytes) */
			inPacketSizes += 5;
		}
	}

	/* HG Data (78 Byte) */
	if (pobTran->srBRec.lnHGTransactionType != 0 &&
			(pobTran->srBRec.uszHappyGoMulti == VS_TRUE || pobTran->srBRec.uszHappyGoSingle == VS_TRUE))
	{
		/* Payment Tools (2 Byte) */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2))
		{
			if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_)
				memcpy(&szDataBuffer[inPacketSizes], "01", 2);
			else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_)
				memcpy(&szDataBuffer[inPacketSizes], "02", 2);
			else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_)
				memcpy(&szDataBuffer[inPacketSizes], "03", 2);
			else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_INSIDE_)
				memcpy(&szDataBuffer[inPacketSizes], "04", 2);
			else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CUP_)
				memcpy(&szDataBuffer[inPacketSizes], "05", 2);
		}

		inPacketSizes += 2;

		/* HG Card Number (18 Byte) */
		/* 重印HG混信用卡的交易簽單不回傳HG卡號 */
		if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2) != 0)
		{
			inRetVal = inECR_CardNoTruncateDecision_HG(pobTran);
			/* 要遮卡號 */
			if (inRetVal == VS_SUCCESS)
			{
				memset(szTemplate, 0x00, sizeof (szTemplate));
				strcpy(szTemplate, pobTran->srBRec.szHGPAN);

				/* HAPPG_GO 卡不掩飾 */
				if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
				{

				} else
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
						for (i = 2; i < (inCardLen - 4); i++)
							szTemplate[i] = '*';
					} else
					{
						for (i = 6; i < (inCardLen - 4); i++)
							szTemplate[i] = '*';
					}

				}

				memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
			} else
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szHGPAN, strlen(pobTran->srBRec.szHGPAN));
			}

		}

		inPacketSizes += 18;

		/* HG Pay Amount (12 Byte) 實際支付金額 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnHGAmount);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		}

		inPacketSizes += 12;
		/* HG Redeem Amount (12 Byte) 扣抵金額 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnHGRedeemAmount);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		}

		inPacketSizes += 12;
		/* HG Redeem Point (8 Byte) 扣抵點數 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGTransactionPoint);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
		}

		inPacketSizes += 8;
		/* HG Lack Point (8 Byte) 不足點數*/
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGRefundLackPoint);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
		}

		inPacketSizes += 8;
		/* HG Balance Point (8 Byte) 剩餘點數 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
				!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGBalancePoint);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
		}

		inPacketSizes += 8;
		/* HG Reserve (10 Byte) */
		inPacketSizes += 10;
	} else
		inPacketSizes += 78;

	return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_Customer_107_Bumper_Pack_ResponseCode
Date&Time       :2016/7/6 下午 2:40
Describe        :先把要送的資料組好
 */
int inECR_8N1_Customer_107_Bumper_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	char szTRTFileName[12 + 1];
	char szFESMode[2 + 1];

	/* 非參加機構卡片判斷 */
	if (!memcmp(pobTran->srBRec.szRespCode, "05", 2) && (!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJB01", 6) ||
			!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJB02", 6) ||
			!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJB03", 6)))
	{
		srECROb->srTransData.inErrorType = _ECR_RESPONSE_CODE_NOT_MEMBER_CARD_;
	}

	if (srECROb->srTransData.inErrorType == VS_CALLBANK)
	{
		memcpy(&szDataBuf[76], "0002", 4);
	} else if (srECROb->srTransData.inErrorType == VS_TIMEOUT ||
			srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_)
	{
		memcpy(&szDataBuf[76], "0003", 4);
	} else if (srECROb->srTransData.inErrorType == VS_USER_OPER_ERR || /* 操作錯誤 & 交易流程有誤 */
			srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_)
	{
		memcpy(&szDataBuf[76], "0004", 4);
	} else if (srECROb->srTransData.inErrorType == VS_ISO_PACK_ERR ||
			srECROb->srTransData.inErrorType == VS_ISO_UNPACK_ERROR ||
			srECROb->srTransData.inErrorType == VS_COMM_ERROR ||
			srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COMM_ERROR_)
	{
		memcpy(&szDataBuf[76], "0005", 4);
	} else if (srECROb->srTransData.inErrorType == VS_USER_CANCEL ||
			srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_)
	{
		memcpy(&szDataBuf[76], "0006", 4);
	} else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_NOT_MEMBER_CARD_)
	{
		memcpy(&szDataBuf[76], "0009", 4); /* 非參加機構卡片 */
	} else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_)
	{
		memcpy(&szDataBuf[76], "0000", 4);
	} else
	{
		memset(szTRTFileName, 0x00, sizeof (szTRTFileName));
		inGetTRTFileName(szTRTFileName);
		/* 2017/9/6 上午 11:21 看verifone code新增1301 */
		if (memcmp(&pobTran->srBRec.szRespCode[0], "00", 2) &&
				memcmp(&pobTran->srBRec.szRespCode[0], "11", 2) &&
				memcmp(szTRTFileName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
		{
			memcpy(&szDataBuf[76], "1301", 4);
		} else
		{
			memcpy(&szDataBuf[76], "0001", 4);
		}
	}

	memset(szFESMode, 0x00, sizeof (szFESMode));
	inGetNCCCFESMode(szFESMode);
	if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)))
	{
		if (pobTran->srBRec.uszMPASTransBit == VS_TRUE)
			memcpy(&szDataBuf[222], "M", 1);
		else
			memcpy(&szDataBuf[222], " ", 1);

		/* MP Response Code */
		if ((memcmp(pobTran->srBRec.szRespCode, "00", 2)) &&
				(!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "CAF", 3) || !memcmp(&pobTran->srBRec.szMPASAuthCode[0], "NEG", 3) ||
				!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "WEB", 3) || !memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJ", 3)))
		{
			memcpy(&szDataBuf[306], &pobTran->srBRec.szMPASAuthCode[0], 6);
		}
	}

	return (VS_SUCCESS);
}

int inECR_SecondReceiveTest(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_FALSE;
	int inEcrTransType = 0;
	unsigned short usLen = 0;
	ECR_TABLE srTempECROb;

	if (inECR_Receive_Check(&usLen) == VS_SUCCESS)
	{
		/* 因為接收資料會清空原本的資料，所以需預先備份原來資料 */
		memset(&srTempECROb, 0x00, sizeof (ECR_TABLE));
		memcpy(&srTempECROb, &gsrECROb, sizeof (ECR_TABLE));

		inRetVal = inECR_Receive_Second_Transaction(pobTran);

		/* 代表ECR已有回傳訊息給POS機，所以要直接中斷交易。 */
		if (gsrECROb.srTransData.uszIsResponce == VS_TRUE)
		{
			inDISP_DispLogAndWriteFlie("  SecondReceive Been Send Msg TransType[%d] LINE[%d]", inEcrTransType, __LINE__);
			return (VS_TRUE);
		}

		//                /* 有正常的接收資料，才進行資料判斷 2022/2/18 [SAM] */
		//                if(inRetVal == VS_SUCCESS)
		//                {
		//                        inEcrTransType = atoi(gsrECROb.srTransData.szTransType);
		//
		//                         /* 只能接授80交易 ，所以如果有收到其它資訊都要中止。 2022/2/18 [SAM] */
		//                        switch (inEcrTransType)
		//                        {
		//                                case _ECR_7E1_MOCHI_PAY_TERMINATE_TRANS_NO_:
		//                                    
		////                                    if(gsrECROb.srTransData.uszIsResponce ==)
		//                                    inDISP_LogPrintfWithFlag("  SecondReceive Terminate[%d] Line[%d]", inEcrTransType, __LINE__);
		//                                    return (VS_TRUE);
		//                                default:
		//                                     /* 因為是不正常的資料，所以要記錄型態是什麼 2022/2/18 [SAM] */
		//                                     pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
		//                                     inDISP_DispLogAndWriteFlie("  SecondReceive *Error* TransType[%d] LINE[%d]", inEcrTransType, __LINE__);
		//                                     return (VS_TRUE);
		//                        }
		//                        
		////                        if(pobTran->uszCardInquiryFirstBit == VS_TRUE)
		////                        {
		////                                switch (inEcrTransType)
		////                                {
		////                                        case _ECR_7E1_MOCHI_PAY_TERMINATE_TRANS_NO_:
		////                                            inDISP_LogPrintfWithFlag("  SecondReceive Terminate[%d] Line[%d]", inEcrTransType, __LINE__);
		////                                            return (VS_TRUE);
		////                                        default:
		////                                             /* 因為是不正常的資料，所以要記錄型態是什麼 2022/2/18 [SAM] */
		////                                             pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
		////                                             inDISP_DispLogAndWriteFlie("  SecondRec *Error* TransType[%d] LINE[%d]", inEcrTransType, __LINE__);
		////                                             return (VS_TRUE);
		////                                }
		////                             
		////                        }else
		////                        {
		////                               /* 不是查詢交易只會接受 80 中斷交易  */
		////                               if(inEcrTransType == _ECR_7E1_MOCHI_PAY_TERMINATE_TRANS_NO_)
		////                               {
		////                                       inDISP_LogPrintfWithFlag("  SecondReceive Terminate[%d] Line[%d]", inEcrTransType, __LINE__);
		////                                       return (VS_TRUE);
		////                               }
		////                        }
		//                
		//                }
		/* 因為二段接收會把中間螢幕清掉，所以要重新顯示刷卡的訊息 2022/2/18 [SAM] */
		/* 顯示對應交易別的感應畫面 */
		inCTLS_Decide_Display_Image(pobTran);
                inCTLS_LED_Wait_Start();

		/* 如果接收的資料不是終止交易或是資料錯誤，需要把原來的ECR傳送資料恢復 2022/2/18 [SAM] */
		memcpy(&gsrECROb, &srTempECROb, sizeof (ECR_TABLE));

	}

	return (VS_FALSE);
}

/*
Function        :inECR_Costco_GasStation_Receive
Date&Time       :2017/11/15 下午 3:14
Describe        :接收收銀機傳來的資料
 */
int inECR_Costco_GasStation_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
	int i;
	int inRetVal;
        int inSize = 0;     /* 變動長度 */
	unsigned short usOneSize = 1; /* 一次只讀一個byte */
	unsigned short usMaxRecLen = _ECR_COSTCO_GASSTATION_MAX_SIZES_;
	unsigned char uszLRC = 0; /* LRC的值 */
	unsigned char uszReceiveBuffer[_ECR_BUFF_SIZE_]; /* 包含STX 和 ETX的原始電文 */
	unsigned char uszSTX = VS_FALSE; /* 是否收到STX */

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

	inFunc_CalculateRunTimeGlobal_Start();

	/* 設定Timeout */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
        
        if(srECROb->srTransDataGas.inFlag_P01 == 0)
        {
                inRetVal = inDISP_Timer_Start(_ECR_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);
        }
        else if(srECROb->srTransDataGas.inFlag_P01 == 1)
        {
                inRetVal = inDISP_Timer_Start(_ECR_RECEIVE_TIMER_, 60);
        }

        /* 復制記號_開始 */
	/* Comport有東西 */
        while(1)
        {
                if(inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
                {
                        return VS_TIMEOUT;
                }
                
                if(inRS232_Data_Receive_Check(srECROb->srSetting.uszComPort, &usOneSize) == VS_SUCCESS)
                {
                        break;
                }
                
        }
	
	usOneSize = 1;
	if (inRS232_Data_Receive(srECROb->srSetting.uszComPort, &uszSTX, &usOneSize) == VS_SUCCESS)
	{
		if (uszSTX == _STX_)
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, 5);
			
			inDISP_LogPrintfWithFlag(" ECR Rec STX Start LINE[%d]",__LINE__);

			uszReceiveBuffer[inSize ++] = _STX_;
                        
			usMaxRecLen -- ;
                        
                        inDISP_LogPrintf("usMaxRecLen A = %d", usMaxRecLen);
                        inDISP_LogPrintf("inSize A = %d", inSize);
                        
			while (1)
			{
				usOneSize = usMaxRecLen;

				inDISP_LogPrintfWithFlag("  ECR Rec Read Len [%u] Line[%d]", usOneSize,  __LINE__);

				if (inRS232_Data_Receive(srECROb->srSetting.uszComPort, &uszReceiveBuffer[inSize], &usOneSize) == VS_SUCCESS)
				{
					inSize += usOneSize;
					usMaxRecLen -= usOneSize;
                                        inDISP_LogPrintf("uszReceiveBuffer[inSize - 2] = %02x", uszReceiveBuffer[inSize - 2]);
                                        inDISP_LogPrintf("inSize = %d", inSize);
					if (uszReceiveBuffer[inSize - 2] == _ETX_)
					{
						/* 算LRC */
                                                inDISP_LogPrintfWithFlag("  ECR Remaining Len [%u][%d] Line[%d]", usMaxRecLen, inSize,  __LINE__);//修正部分卡片無法感應的問題 add by sampo 20201216
                                                
                                                for (i = 1; i < (inSize-1); i++)
                                                {
                                                        uszLRC ^= uszReceiveBuffer[i];
                                                }
              
						break;
					}

				}

				if (inTimerGet(_TIMER_NEXSYS_2_) == 0)
				{
					inDISP_LogPrintfWithFlag("  Multi Host Rec Data Aft EXT Timout *Error* Line[%d]", __LINE__);
					return (VS_TIMEOUT);
				}

			}
                        
                        inDISP_LogPrintf("Miyano Flag uszLRCData = %02x", uszLRC);
                        inDISP_LogPrintf("Miyano Flag uszRecBuf[inSize - 1] = %02x", uszReceiveBuffer[inSize - 1]);
                        
			/* 驗證LRC */
			if (uszLRC == uszReceiveBuffer[inSize - 1])
			{
				inDISP_LogPrintfWithFlag("  Multi Host Rec LRC OK Line[%d]", __LINE__);
			}
			else
			{
				inDISP_LogPrintfWithFlag("  Multi Host Rec LRC NOT OK Line[%d]", __LINE__);
				return (VS_FAILURE);
			}
		}
		else
		{
			inDISP_LogPrintfWithFlag("  Multi Host Rec Not Found STX Val[0x%x] Line[%d]", uszSTX, __LINE__ );
			return (VS_WAVE_NO_DATA);
		}
	}
	else
	{		
		inDISP_LogPrintfWithFlag("-----[%s][%s][%d]  No Data *Error* END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_WAVE_NO_DATA);
	}
        /* 復制記號_結尾 */
        
	/* 去除STX、ETX、LRC，把資料放到szrRealReceBuffer */
	memcpy(szDataBuffer, &uszReceiveBuffer[1], inSize - 3);
        
        if(ginDebug == VS_TRUE)
        {
                inDISP_LogPrintf("Miyano Flag Test B COSTCO ECR: %s", uszReceiveBuffer);
        }

	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Receive_ISODeBug(szDataBuffer, inSize - 3, inSize);

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);

	return (VS_SUCCESS);
}

int inECR_Costco_GasStation_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        int     i = 0;
        int     inTransType = 0;
        int     inLenDataTmp = 0;
        char    szDataTmp[100] = {0};   
        
        inDISP_LogPrintf("inECR_Costco_GasStation_Unpack Start");
        
        inDISP_LogPrintf("szDataBuffer = %s", szDataBuffer);
        memset((char *)&srECROb->srTransDataGas, 0x00, sizeof(srECROb->srTransDataGas));
        
        /* 先取第一個逗號之前的值 ex:, "P01","P02" */
        inLenDataTmp = inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szCommand, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szCommand = %s\n", srECROb->srTransDataGas.szCommand);

        if(!memcmp(srECROb->srTransDataGas.szCommand, "P01", inLenDataTmp))
        {
                inDISP_LogPrintf("_ECR_Costco_GasStation_Poll_");
                inECR_Costco_GasStation_Unpack_Poll(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P02", inLenDataTmp))
        {
                inDISP_LogPrintf("_ECR_Costco_GasStation_AuthComplete_");
                inECR_Costco_GasStation_Unpack_AuthComplete(pobTran, srECROb, szDataBuffer);                
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P04", inLenDataTmp))
        {
                inDISP_LogPrintf("_ECR_Costco_GasStation_ChangeMode_");
                inECR_Costco_GasStation_Unpack_ChangeMode(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P06", inLenDataTmp))
        {
                inDISP_LogPrintf("_ECR_Costco_GasStation_ConfigSetting_");
                memset((char *)gsrECR_GASStation_tb.szPumpNo, 0x00, sizeof(gsrECR_GASStation_tb.szPumpNo));
                inECR_Costco_GasStation_Unpack_ConfigSetting(pobTran, srECROb, szDataBuffer);     
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P07", inLenDataTmp))
        {
                inDISP_LogPrintf("_ECR_Costco_GasStation_Inquery_");
                inECR_Costco_GasStation_Unpack_Inquery(pobTran, srECROb, szDataBuffer);     
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P08", inLenDataTmp))
        {
                inDISP_LogPrintf("_ECR_Costco_GasStation_Inquery_CostcoPay");
                inECR_Costco_GasStation_Unpack_Inquery_CostcoPay(pobTran, srECROb, szDataBuffer);      
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P10", inLenDataTmp))
        {
                inDISP_LogPrintf("_ECR_Costco_GasStation_ShowData_");
                inECR_Costco_GasStation_Unpack_ShowData(pobTran, srECROb, szDataBuffer);      
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P99", inLenDataTmp))
        {
                inDISP_LogPrintf("_ECR_Costco_GasStation_SVC_");
                inECR_Costco_GasStation_Unpack_SVC(pobTran, srECROb, szDataBuffer);    
        }
        
        inDISP_LogPrintf("inECR_Costco_GasStation_Unpack End");
        return (VS_SUCCESS);
}

int inECR_Costco_GasStation_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        char szTMSOK[2 + 1] = {0};
        
        inDISP_LogPrintf("inECR_Costco_GasStation_Pack Start");
        
        if(!memcmp(srECROb->srTransDataGas.szCommand, "Busy", 4))
        {
                inDISP_LogPrintf("Busy");
                memset(gsrECR_GASStation_tb.szEDCStatus, 0x00, sizeof(gsrECR_GASStation_tb.szEDCStatus));
                memcpy(gsrECR_GASStation_tb.szEDCStatus, "B", 1);
                inECR_Costco_GasStation_Pack_EDCStatus(pobTran, srECROb, szDataBuffer);
                return (VS_SUCCESS);
        }
        
        /* 再根據第一段指令選擇執行要Pack的函式 */
        if(!memcmp(srECROb->srTransDataGas.szCommand, "P01", 3))
        {
                inDISP_LogPrintf("P01");
                inECR_Costco_GasStation_Pack_EDCStatus(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P02", 3))
        {
                inDISP_LogPrintf("P02");
                memset(gsrECR_GASStation_tb.szEDCStatus, 0x00, sizeof(gsrECR_GASStation_tb.szEDCStatus));
                memcpy(gsrECR_GASStation_tb.szEDCStatus, "K", 1);
                memset(gsrECR_GASStation_tb.uszErrorCode, 0x00, sizeof(gsrECR_GASStation_tb.uszErrorCode));
                memcpy(gsrECR_GASStation_tb.uszErrorCode, "N", 1);
                inECR_Costco_GasStation_Pack_EDCStatus(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P04", 3))
        {
                inDISP_LogPrintf("P04");
                memset(gsrECR_GASStation_tb.szEDCStatus, 0x00, sizeof(gsrECR_GASStation_tb.szEDCStatus));
                memcpy(gsrECR_GASStation_tb.szEDCStatus, "K", 1);
                memset(gsrECR_GASStation_tb.uszErrorCode, 0x00, sizeof(gsrECR_GASStation_tb.uszErrorCode));
                memcpy(gsrECR_GASStation_tb.uszErrorCode, "N", 1);
                inECR_Costco_GasStation_Pack_EDCStatus(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P06", 3))
        {
                inDISP_LogPrintf("P06");
                /* EDC狀態 */
                /* 如果鎖機或TMSOK不是'Y'，回傳E(Error)，正常回傳K(ack) */
                memset(szTMSOK, 0x00, sizeof (szTMSOK));
                inGetTMSOK(szTMSOK);
                if ((inFunc_Check_EDCLock() != VS_SUCCESS) || (memcmp(szTMSOK, "Y", 1) != 0))
                {
                        memset(gsrECR_GASStation_tb.szEDCStatus, 0x00, sizeof(gsrECR_GASStation_tb.szEDCStatus)); 
                        memcpy(gsrECR_GASStation_tb.szEDCStatus, "E", 1);
                        memset(gsrECR_GASStation_tb.uszErrorCode, 0x00, sizeof(gsrECR_GASStation_tb.uszErrorCode));
                        memcpy(gsrECR_GASStation_tb.uszErrorCode, "EDC Error", 9);
                }
                else
                {
                        memset(gsrECR_GASStation_tb.szEDCStatus, 0x00, sizeof(gsrECR_GASStation_tb.szEDCStatus));
                        memcpy(gsrECR_GASStation_tb.szEDCStatus, "K", 1);
                        memset(gsrECR_GASStation_tb.uszErrorCode, 0x00, sizeof(gsrECR_GASStation_tb.uszErrorCode));
                        memcpy(gsrECR_GASStation_tb.uszErrorCode, "N", 1);
                }
                inECR_Costco_GasStation_Pack_EDCStatus(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P08", 3))
        {
                inDISP_LogPrintf("P08");
                inECR_Costco_GasStation_Pack_EDCStatus(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P99", 3))
        {
                inDISP_LogPrintf("P99");
        }

        inDISP_LogPrintf("inECR_Costco_GasStation_Pack End");
        return (VS_SUCCESS);
}


/*  2023.03.16
 *  Miyano
 *  szResult 儲存取出的資料, szDataBuffer 原始Data
 *  取 '逗號' 或 '0x00' 之前的資料
 *  成功回傳 資料長度
*/
int inECR_Costco_Get_DataSegment(char *szResult, char *szDataBuffer)
{
        int     i = 0;
        VS_BOOL fFinal = VS_FALSE;    
                
        for(i=0;;i++)
        {
                if(szDataBuffer[i] == ',' || i == strlen(szDataBuffer))
                {
                        if(i == strlen(szDataBuffer))
                        {
                                fFinal = VS_TRUE;
                        }
                        break;
                }
                else
                {
                        szResult[i] = szDataBuffer[i];
                }
        }
        
        if(fFinal != VS_TRUE)
        {
                inECR_Costco_CutData(strlen(szResult)+1, &szDataBuffer[0]);
        }
        
        return (strlen(szResult));
}

int inECR_Costco_CutData(int inCut, char *szDataBuffer)
{
        int i = 0;
        int inTmpStrlen = 0;
        
        memcpy(&szDataBuffer[0], &szDataBuffer[inCut], strlen(szDataBuffer) - inCut);
        
        inTmpStrlen = strlen(szDataBuffer);

        for(i=0;i<(inCut);i++)
        {
                szDataBuffer[inTmpStrlen - inCut + i] = 0x00;
        }

        return  (VS_SUCCESS);
}

int inECR_Costco_GasStation_Unpack_Poll(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        int     i = 0;
        int     inTransType = 0;
        int     inLenDataTmp = 0;
        char    szDataTmp[100] = {0};   
        
        /* Pump 號碼 */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szPumpNo, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szPumpNo = %s\n", srECROb->srTransDataGas.szPumpNo);

        /* Pump 狀態 */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szPumpStatus, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szPumpStatus = %s\n", srECROb->srTransDataGas.szPumpStatus);

                
        return  (VS_SUCCESS);
}

int inECR_Costco_GasStation_Unpack_AuthComplete(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        int     i = 0;
        int     inTransType = 0;
        int     inLenDataTmp = 0;
        char    szDataTmp[100] = {0};   
        
        /* Pump 號碼 */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szPumpNo, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szPumpNo = %s\n", srECROb->srTransDataGas.szPumpNo);

        /* 卡號 */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szPan, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szPAN = %s\n", srECROb->srTransDataGas.szPan);

        /* Approve No. */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szApproveNo, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szApproveNo = %s\n", srECROb->srTransDataGas.szApproveNo);

        /* Unit Price */
        memset(szDataTmp, 0x00, sizeof(szDataTmp));
        inLenDataTmp = inECR_Costco_Get_DataSegment(szDataTmp, szDataBuffer);
        srECROb->srTransDataGas.fUnitPrice = atof(szDataTmp);
        inDISP_LogPrintf("Miyano Flag fUnitPrice = %.2f\n", srECROb->srTransDataGas.fUnitPrice);

        /* 數量 */
        memset(szDataTmp, 0x00, sizeof(szDataTmp));
        inLenDataTmp = inECR_Costco_Get_DataSegment(szDataTmp, szDataBuffer);
        srECROb->srTransDataGas.fQunity = atof(szDataTmp);
        inDISP_LogPrintf("Miyano Flag fQunity = %.3f\n", srECROb->srTransDataGas.fQunity);

        /* 金額 */
        memset(szDataTmp, 0x00, sizeof(szDataTmp));
        inLenDataTmp = inECR_Costco_Get_DataSegment(szDataTmp, szDataBuffer);
        srECROb->srTransDataGas.lnAmount = atol(szDataTmp);
        inDISP_LogPrintf("Miyano Flag lnAmount = %ld\n", srECROb->srTransDataGas.lnAmount);

        /* 折扣金額 */
        memset(szDataTmp, 0x00, sizeof(szDataTmp));
        inLenDataTmp = inECR_Costco_Get_DataSegment(szDataTmp, szDataBuffer);
        srECROb->srTransDataGas.lnDiscount = atol(szDataTmp);
        inDISP_LogPrintf("Miyano Flag lnDiscount = %ld\n", srECROb->srTransDataGas.lnDiscount);

        /* 調閱編號 */
        inLenDataTmp = inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szInvoice, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szInvoice = %s\n", srECROb->srTransDataGas.szInvoice);

        /* 批次編號 */
        inLenDataTmp = inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szBatch, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szBatch = %s\n", srECROb->srTransDataGas.szBatch);

        /* 過卡方式 */
        inLenDataTmp = inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szWayPassCard, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szWayPassCard = %s\n", srECROb->srTransDataGas.szWayPassCard);
                
        return  (VS_SUCCESS);
}

int inECR_Costco_GasStation_Unpack_ChangeMode(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        int     i = 0;
        int     inTransType = 0;
        int     inLenDataTmp = 0;
        char    szDataTmp[100] = {0};   
        
        /* Pump 號碼 */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szPumpNo, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szPumpNo = %s\n", srECROb->srTransDataGas.szPumpNo);

        /* 要切換的模式(讀卡、結帳、掃碼...) */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szEDCMode, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szEDCMode = %s\n", srECROb->srTransDataGas.szEDCMode);

        /* 其他陳述1 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement1, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement1 = %s\n", srECROb->srTransDataGas.uszArgement1);

        /* 其他陳述2 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement2, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag fUnitPrice = %.2f\n", srECROb->srTransDataGas.uszArgement2);

        return  (VS_SUCCESS);
}

int inECR_Costco_GasStation_Unpack_ConfigSetting(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        int     i = 0;
        int     inTransType = 0;
        int     inLenDataTmp = 0;
        char    szDataTmp[100] = {0};   
        
        /* Pump 號碼 */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szPumpNo, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szPumpNo = %s\n", srECROb->srTransDataGas.szPumpNo);
        
        memset(gsrECR_GASStation_tb.szPumpNo, 0x00, sizeof(gsrECR_GASStation_tb.szPumpNo));
        memcpy((char *)gsrECR_GASStation_tb.szPumpNo,
                srECROb->srTransDataGas.szPumpNo, strlen(srECROb->srTransDataGas.szPumpNo));
        
        inDISP_LogPrintf("gsrECR_GASStation_tb.szPumpNo = %s\n", gsrECR_GASStation_tb.szPumpNo);

        /* 其他陳述1 */
        /* 這個會出現',' 所以用memcpy把剩下的全部貼過來 */
        memcpy((char *)srECROb->srTransDataGas.uszArgement1, szDataBuffer, strlen(szDataBuffer));
        inDISP_LogPrintf("Miyano Flag uszArgement1 = %s\n", srECROb->srTransDataGas.uszArgement1);

        return  (VS_SUCCESS);
}

int inECR_Costco_GasStation_Unpack_Inquery(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        int     i = 0;
        int     inTransType = 0;
        int     inLenDataTmp = 0;
        char    szDataTmp[100] = {0};   
        
        /* Pump 號碼 */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szPumpNo, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szPumpNo = %s\n", srECROb->srTransDataGas.szPumpNo);
        
        /* 要切換的模式(讀卡、結帳、掃碼...) */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szEDCMode, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szEDCMode = %s\n", srECROb->srTransDataGas.szEDCMode);
        
        /* 其他陳述1 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement1, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement1 = %s\n", srECROb->srTransDataGas.uszArgement1);
        
        /* 其他陳述2 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement2, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement2 = %s\n", srECROb->srTransDataGas.uszArgement2);

        return  (VS_SUCCESS);
}

int inECR_Costco_GasStation_Unpack_Inquery_CostcoPay(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        int     i = 0;
        int     inTransType = 0;
        int     inLenDataTmp = 0;
        char    szDataTmp[100] = {0};   
        
        /* Pump 號碼 */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szPumpNo, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szPumpNo = %s\n", srECROb->srTransDataGas.szPumpNo);
        
        /* 要切換的模式(讀卡、結帳、掃碼...) */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szEDCMode, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szEDCMode = %s\n", srECROb->srTransDataGas.szEDCMode);
        
        /* 其他陳述1 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement1, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement1 = %s\n", srECROb->srTransDataGas.uszArgement1);
        
        /* 其他陳述2 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement2, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement2 = %s\n", srECROb->srTransDataGas.uszArgement2);

        return  (VS_SUCCESS);
}

int inECR_Costco_GasStation_Unpack_ShowData(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        int     i = 0;
        int     inTransType = 0;
        int     inLenDataTmp = 0;
        char    szDataTmp[100] = {0};   
        
        /* Pump 號碼 */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szPumpNo, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szPumpNo = %s\n", srECROb->srTransDataGas.szPumpNo);
        
        /* 其他陳述1 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement1, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement1 = %s\n", srECROb->srTransDataGas.uszArgement1);
        
        /* 其他陳述2 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement2, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement2 = %s\n", srECROb->srTransDataGas.uszArgement2);
        
        /* 其他陳述3 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement3, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement3 = %s\n", srECROb->srTransDataGas.uszArgement3);
        
        /* 其他陳述4 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement4, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement4 = %s\n", srECROb->srTransDataGas.uszArgement4);

        return  (VS_SUCCESS);
}

int inECR_Costco_GasStation_Unpack_SVC(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        int     i = 0;
        int     inTransType = 0;
        int     inLenDataTmp = 0;
        char    szDataTmp[100] = {0};   
        
        /* Pump 號碼 */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szPumpNo, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szPumpNo = %s\n", srECROb->srTransDataGas.szPumpNo);
        
        /* 要執行的SVC相關功能 */
        inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szSVCType, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szType = %s\n", srECROb->srTransDataGas.szSVCType);
        
        /* 其他陳述1 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement1, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement1 = %s\n", srECROb->srTransDataGas.uszArgement1);
        
        /* 其他陳述2 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement2, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement2 = %s\n", srECROb->srTransDataGas.uszArgement2);
        
        /* 其他陳述3 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement3, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement3 = %s\n", srECROb->srTransDataGas.uszArgement3);
        
        /* 其他陳述4 */
        inECR_Costco_Get_DataSegment((char *)srECROb->srTransDataGas.uszArgement4, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag uszArgement4 = %s\n", srECROb->srTransDataGas.uszArgement4);

        return  (VS_SUCCESS);
}

int inECR_Costco_GasStation_Pack_EDCStatus(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        char szComma[1+1] = {0};
        
        memset(szComma, 0x00, sizeof(szComma));
        memcpy(szComma, ",", 1);
        
        /* E01 */
        strcpy(szDataBuffer, "E01,");
        inDISP_LogPrintf("Miyano Flag1 szDataBuffer = %s\n", szDataBuffer);
        /* 加油泵號碼 */
        strcat(szDataBuffer, gsrECR_GASStation_tb.szPumpNo);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag2 szDataBuffer = %s\n", szDataBuffer);
        /* EDC狀態 */
        strcat(szDataBuffer, gsrECR_GASStation_tb.szEDCStatus);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag4 szDataBuffer = %s\n", szDataBuffer);
        /* EDC 錯誤碼 */
        strcat(szDataBuffer, gsrECR_GASStation_tb.uszErrorCode);
        inDISP_LogPrintf("Miyano Flag5 szDataBuffer = %s\n", szDataBuffer);
        
        return (VS_SUCCESS);
}

/* 20230419 Miyano add 專解poll */
int inECR_Costco_GasStation_Unpack_P01(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        int     i = 0;
        int     inTransType = 0;
        int     inLenDataTmp = 0;
        int     inRetVal = 0;
        char    szDataTmp[100] = {0};   
        
        inDISP_LogPrintf("inECR_Costco_GasStation_Unpack_P01 Start");
        
        inDISP_LogPrintf("szDataBuffer = %s", szDataBuffer);
        memset((char *)&srECROb->srTransDataGas, 0x00, sizeof(srECROb->srTransDataGas));
        
        /* 先取第一個逗號之前的值 ex:, "P01","P02" */
        inLenDataTmp = inECR_Costco_Get_DataSegment(srECROb->srTransDataGas.szCommand, szDataBuffer);
        inDISP_LogPrintf("Miyano Flag szCommand = %s\n", srECROb->srTransDataGas.szCommand);

        if(!memcmp(srECROb->srTransDataGas.szCommand, "P01", inLenDataTmp))
        {
                inDISP_LogPrintf("_ECR_Costco_GasStation_Poll_");
                inECR_Costco_GasStation_Unpack_Poll(pobTran, srECROb, szDataBuffer);
                /* 暫時，不知道有什麼條件，先收到P01就回傳SUCCESS */
                inRetVal = VS_SUCCESS;
        }
        else
        {
                inDISP_LogPrintf("Not Poll");
                inRetVal = VS_ERROR;
        }
        inDISP_LogPrintf("inECR_Costco_GasStation_Unpack_P01 End");
        return inRetVal;
}

int inECR_Costco_GasStation_Pack2(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        char szTMSOK[2 + 1] = {0};
        
        inDISP_LogPrintf("inECR_Costco_GasStation_Pack2 Start");
        
        /* 如果是需要Initial或EDC Busy，要回復狀態並Return */
        /* 不能取代原來的指令，因為當ECR發正確指令後，要能繼續進行交易 */
        if(!memcmp(srECROb->srTransDataGas.szCommand, "Initial", 7))
        {
                inDISP_LogPrintf("Initial");
                memset(gsrECR_GASStation_tb.szEDCStatus, 0x00, sizeof(gsrECR_GASStation_tb.szEDCStatus));
                memcpy(gsrECR_GASStation_tb.szEDCStatus, "R", 1);
                inECR_Costco_GasStation_Pack_EDCStatus(pobTran, srECROb, szDataBuffer);
                return (VS_SUCCESS);
        }
        
        /* 根據指令選擇執行要Pack的函式 */
        if(!memcmp(gsrECR_GASStation_tb.szCommand, "P01", 3))
        {
                inDISP_LogPrintf("P01");
                inECR_Costco_GasStation_Pack_EDCStatus(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(gsrECR_GASStation_tb.szCommand, "P06", 3))
        {
                inDISP_LogPrintf("P06");
                inECR_Costco_GasStation_Pack_EDCStatus(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(gsrECR_GASStation_tb.szCommand, _Costco_szCommand_ReadCard_, 2))   /* 過卡 */
        {
                inDISP_LogPrintf("ReadCard");
                inECR_Costco_GasStation_Pack_ReadCard(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(gsrECR_GASStation_tb.szCommand, _Costco_szCommand_PreAuth_, 2))   /* 預先授權 */
        {
                inDISP_LogPrintf("PreAuth");
                inECR_Costco_GasStation_Pack_PreAuth(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(gsrECR_GASStation_tb.szCommand, _Costco_szCommand_AuthComplete_, 2))   /* 授權完成 */
        {
                inDISP_LogPrintf("AuthComplete");
                inECR_Costco_GasStation_Pack_AuthComplete(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(gsrECR_GASStation_tb.szCommand, _Costco_szCommand_Settle_, 2))   /* 結帳 */
        {
                inDISP_LogPrintf("Settle");
                inECR_Costco_GasStation_Pack_Settle(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(gsrECR_GASStation_tb.szCommand, _Costco_szCommand_PreAuth_Complete_Err, 2))   /* 預先授權或授權完成失敗 */
        {
                inDISP_LogPrintf("PreAuth_Complete_Err");
                inECR_Costco_GasStation_Pack_PreAuth_Complete_ERR(pobTran, srECROb, szDataBuffer);
        }
        else if(!memcmp(gsrECR_GASStation_tb.szCommand, _Costco_szCommand_ScanPay_, 2))   /* 預先授權或授權完成失敗 */
        {
                inDISP_LogPrintf("ScanPay");
                inECR_Costco_GasStation_Pack_ScanPay(pobTran, srECROb, szDataBuffer);
        }
        

        inDISP_LogPrintf("inECR_Costco_GasStation_Pack2 End");
        return (VS_SUCCESS);
}

int inECR_Costco_GasStation_Pack_ReadCard(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        char szComma[1+1] = {0};
        
        memset(szComma, 0x00, sizeof(szComma));
        memcpy(szComma, ",", 1);
        
        /* E01 */
        strcpy(szDataBuffer, "E05,");
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 加油泵號碼 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szPumpNo);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* Response Code */
        strcat(szDataBuffer, srECROb->srTransDataGas.szResponse);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 暫時，卡別 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szCardType);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 卡號 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szPan);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 卡片有效期 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szCardExpDate);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 暫時，Preset Amount */
        strcat(szDataBuffer, srECROb->srTransDataGas.szPresetAmount);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 暫時，Transaction Type */
        strcat(szDataBuffer, srECROb->srTransDataGas.szTransactionType);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 卡片種類 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szCardKind);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 過卡方式 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szWayPassCard);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);

        return (VS_SUCCESS);
}

int inECR_Costco_GasStation_Pack_PreAuth(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        char szComma[1+1] = {0};
        char szTmpBuff[100+1] = {0};
        
        inDISP_LogPrintf("inECR_Costco_GasStation_Pack_PreAuth Start");
        
        memset(szComma, 0x00, sizeof(szComma));
        memcpy(szComma, ",", 1);
        
        /* E02 */
        strcpy(szDataBuffer, "E02,");
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 加油泵號碼 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szPumpNo);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* Response Code */
        strcat(szDataBuffer, srECROb->srTransDataGas.szResponse);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* Card Type */
        strcat(szDataBuffer, srECROb->srTransDataGas.szCardType);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 卡號 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szPan);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 卡片有效期 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szCardExpDate);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 暫時，Present Amount */
        strcat(szDataBuffer, srECROb->srTransDataGas.szPresetAmount);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 暫時，Transaction Type */
        strcat(szDataBuffer, srECROb->srTransDataGas.szTransactionType);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* Trans Amount */
        memset(szTmpBuff, 0x00, sizeof(szTmpBuff));
        sprintf(szTmpBuff, "%ld", srECROb->srTransDataGas.lnAmount);
        strcat(szDataBuffer, szTmpBuff);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 授權碼 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szApproveNo);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* TID */
        strcat(szDataBuffer, srECROb->srTransDataGas.szTID);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* MID */
        strcat(szDataBuffer, srECROb->srTransDataGas.szMID);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 批次號碼 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szBatch);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 調閱編號 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szInvoice);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 城市名 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szCity);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 卡片種類 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szCardKind);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 過卡方式 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szWayPassCard);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* TxRespCode */
        strcat(szDataBuffer, srECROb->srTransDataGas.szTxRespCode);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 暫時，看起來沒寫加油站，待補，電子發票載具 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szEInvoiceCarrier);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* TVR */
        strcat(szDataBuffer, srECROb->srTransDataGas.usz95_TVR);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* TSI */
        strcat(szDataBuffer, srECROb->srTransDataGas.usz9B_TSI);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* AIP */
        strcat(szDataBuffer, srECROb->srTransDataGas.usz82_AIP);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* CID */
        strcat(szDataBuffer, srECROb->srTransDataGas.usz9F27_CID);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 9F26 */
        strcat(szDataBuffer, srECROb->srTransDataGas.usz9F26_ApplCryptogram);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
       
        inDISP_LogPrintf("inECR_Costco_GasStation_Pack_PreAuth End");
        return (VS_SUCCESS);
}

int inECR_Costco_GasStation_Pack_AuthComplete(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        char szComma[1+1] = {0};
        char szTmpBuff[100+1] = {0};
        
        inDISP_LogPrintf("inECR_Costco_GasStation_Pack_AuthComplete Start");
        
        memset(szComma, 0x00, sizeof(szComma));
        memcpy(szComma, ",", 1);
        
        /* E03 */
        strcpy(szDataBuffer, "E03,");
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 加油泵號碼 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szPumpNo);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* Response Code */
        strcat(szDataBuffer, srECROb->srTransDataGas.szResponse);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* Card Type */
        strcat(szDataBuffer, srECROb->srTransDataGas.szCardType);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 卡號 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szPan);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 卡片有效期 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szCardExpDate);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 暫時，Present Amount */
        strcat(szDataBuffer, srECROb->srTransDataGas.szPresetAmount);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 暫時，Transaction Type */
        strcat(szDataBuffer, srECROb->srTransDataGas.szTransactionType);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* Trans Amount */
        memset(szTmpBuff, 0x00, sizeof(szTmpBuff));
        sprintf(szTmpBuff, "%ld", srECROb->srTransDataGas.lnAmount);
        strcat(szDataBuffer, szTmpBuff);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 授權碼 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szApproveNo);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* TID */
        strcat(szDataBuffer, srECROb->srTransDataGas.szTID);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* MID */
        strcat(szDataBuffer, srECROb->srTransDataGas.szMID);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 批次號碼 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szBatch);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 調閱編號 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szInvoice);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 城市名 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szCity);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 卡片種類 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szCardKind);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 過卡方式 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szWayPassCard);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* TVR */
        strcat(szDataBuffer, srECROb->srTransDataGas.usz95_TVR);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* TSI */
        strcat(szDataBuffer, srECROb->srTransDataGas.usz9B_TSI);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* AIP */
        strcat(szDataBuffer, srECROb->srTransDataGas.usz82_AIP);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* CID */
        strcat(szDataBuffer, srECROb->srTransDataGas.usz9F27_CID);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 9F26 */
        strcat(szDataBuffer, srECROb->srTransDataGas.usz9F26_ApplCryptogram);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
       
        inDISP_LogPrintf("inECR_Costco_GasStation_Pack_AuthComplete End");
        return (VS_SUCCESS);
}

int inECR_Costco_GasStation_Pack_PreAuth_Complete_ERR(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        char szComma[1+1] = {0};
        char szTmpBuff[100+1] = {0};
        
        inDISP_LogPrintf("inECR_Costco_GasStation_Pack_PreAuth_Complete_ERR Start");
        
        memset(szComma, 0x00, sizeof(szComma));
        memcpy(szComma, ",", 1);
        
        /* E02 */
        strcpy(szDataBuffer, "E02,");
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 加油泵號碼 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szPumpNo);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* Response Code */
        strcat(szDataBuffer, srECROb->srTransDataGas.szResponse);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* Error String */
        strcat(szDataBuffer, srECROb->srTransDataGas.uszErrString);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
 
        inDISP_LogPrintf("inECR_Costco_GasStation_Pack_PreAuth_Complete_ERR End");
        return (VS_SUCCESS);
}

int inECR_Costco_GasStation_Pack_Settle(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        char szComma[1+1] = {0};
        char szTmpBuff[100+1] = {0};
        
        inDISP_LogPrintf("inECR_Costco_GasStation_Pack_PreAuth_Complete_ERR Start");
        
        memset(szComma, 0x00, sizeof(szComma));
        memcpy(szComma, ",", 1);
        
        /* E06 */
        strcpy(szDataBuffer, "E06,");
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 加油泵號碼 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szPumpNo);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* Response Code */
        strcat(szDataBuffer, srECROb->srTransDataGas.szResponse);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        
        /* Date */
        strcat(szDataBuffer, srECROb->srTransDataGas.szDate);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        
        /* Time */
        strcat(szDataBuffer, srECROb->srTransDataGas.szTime);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        
        /* TID */
        strcat(szDataBuffer, srECROb->srTransDataGas.szTID);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        
        /* MID */
        strcat(szDataBuffer, srECROb->srTransDataGas.szMID);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        
        /* 交易總筆數 */
        memset(szTmpBuff, 0x00, sizeof(szTmpBuff));
        sprintf(szTmpBuff, "%d", srECROb->srTransDataGas.inTransTotalCount);
        strcat(szDataBuffer, szTmpBuff);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 交易總金額 */
        memset(szTmpBuff, 0x00, sizeof(szTmpBuff));
        sprintf(szTmpBuff, "%d", srECROb->srTransDataGas.inTransTotalAmount);
        strcat(szDataBuffer, szTmpBuff);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);    
 
        inDISP_LogPrintf("inECR_Costco_GasStation_Pack_PreAuth_Complete_ERR End");
        return (VS_SUCCESS);
}

/* 暫時，看不懂規格書 */
int inECR_Costco_GasStation_Pack_ScanPay(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, char *szDataBuffer)
{
        char szComma[1+1] = {0};
        char szTmpBuff[100+1] = {0};
        
        inDISP_LogPrintf("inECR_Costco_GasStation_Pack_ScanPay Start");
        
        memset(szComma, 0x00, sizeof(szComma));
        memcpy(szComma, ",", 1);
        
        /* E06 */
        strcpy(szDataBuffer, "E08,");
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        /* 加油泵號碼 */
        strcat(szDataBuffer, srECROb->srTransDataGas.szPumpNo);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);

        /* Date */
        strcat(szDataBuffer, srECROb->srTransDataGas.szDate);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        
        /* Time */
        strcat(szDataBuffer, srECROb->srTransDataGas.szTime);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        
        /* TID */
        strcat(szDataBuffer, srECROb->srTransDataGas.szTID);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);
        
        /* MID */
        strcat(szDataBuffer, srECROb->srTransDataGas.szMID);
        strcat(szDataBuffer, szComma);
        inDISP_LogPrintf("Miyano Flag szDataBuffer = %s\n", szDataBuffer);

        inDISP_LogPrintf("inECR_Costco_GasStation_Pack_ScanPay End");
        return (VS_SUCCESS);
}