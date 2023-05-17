#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sqlite3.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/ECR_FUNC/ECR.h"
#include "../SOURCE/FUNCTION/ECCDT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/FILE_FUNC/File.h"
#include "../SOURCE/FUNCTION/XML.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/COMM/Ethernet.h"
#include "../ECC/ECC.h"


#include "CMASFunction.h"
#include "CMASAPIISO.h"

extern int ginDebug; /* Debug使用 extern */
extern unsigned char guszECCRetryBit;
extern ECC_RETRY_DATA   gszECCRetryData;
extern int ginAPVersionType ;
extern int ginCMASDirLink;

//extern szEzCardBlance;

/*
Function        :inCMAS_PACK_ICERINI
Date&Time       :2020/2/5 上午 11:05
Describe        :組悠遊卡參數XML
Author          :Hachi
 */
int inCMAS_PACK_ICERINI(TRANSACTION_OBJECT *pobTran)
{
	char szHostIndex[2 + 1] = {0};
	char szCommunicationIndex[2 + 1] = {0};
	char szHostIP[16 + 1] = {0};
	char szPort[5 + 1] = {0};
	//	char		szECC_SP_ID[8 + 1] = {0};
	char szMerchantID[15 + 1] = {0};
	char szSAMSlot[2 + 1] = {0};
	char szIFESMode[2 + 1] = {0};
	char szTemplate[50 + 1] = {0};
	// char szCustomerIndicator[3 + 1] = {0};
	// char szFuncEnable[2 + 1] = {0};
	// char szCFESMode[2 + 1] = {0};
	// char szNCCCMode[2 + 1] = {0};
	char szCMASAddTcpipMode[ 2 + 1 ] ={0};
	// char szVersionID [2 + 1] = {0};
	xmlDocPtr srDoc = NULL;
	xmlNodePtr srRoot_Node = NULL;

	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
		return (VS_ERROR);

	/* MFT index 紀錄 HDT index */
	memset(szHostIndex, 0x00, sizeof(szHostIndex));
	inGetTicket_HostIndex(szHostIndex);
	pobTran->srBRec.inHDTIndex = atoi(szHostIndex);

	if (inLoadEDCRec(0)!= VS_SUCCESS)
		return (VS_ERROR);
	if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
		return (VS_ERROR);
	if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
		return (VS_ERROR);

	/* 刪除原本的ICERINI */
	inFile_Unlink_File(_CMAS_PARA_FILE_, "");

	/* 建立一個新的檔案 */
	srDoc = xmlNewDoc(BAD_CAST "1.0");
	srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
	xmlDocSetRootElement(srDoc, srRoot_Node);

	/* LogFlag */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "LogFlag", BAD_CAST "1");

	/* DLLVersion */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "DLLVersion", BAD_CAST "2");

	/* TCPIPTimeOut */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "TCPIPTimeOut", BAD_CAST "30");

	/* LogCnt */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "LogCnt", BAD_CAST "30");

	/* ComPort */
	/*問過儒勳，因為是內建，所以ComPort設什麼值都不影響*/
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ComPort", BAD_CAST "1");

	/* ECC_IP */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ECC_IP", BAD_CAST "172.16.11.20");

	/* ECC_Port */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ECC_Port", BAD_CAST "8902");

	/* ICER_IP */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ICER_IP", BAD_CAST "172.25.17.95");

	/* ICER_Port */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ICER_Port", BAD_CAST "8303");

	/* CMAS_IP *//*這是主機ip*/
	memset(szCommunicationIndex, 0x00, sizeof(szCommunicationIndex));
	inGetCommunicationIndex(szCommunicationIndex);
	inLoadCPTRec(atoi(szCommunicationIndex) - 1);
	memset(szHostIP, 0x00, sizeof(szHostIP));
	inGetHostIPPrimary(szHostIP);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "CMAS_IP", BAD_CAST szHostIP);

	/* CMAS_Port */
	memset(szPort, 0x00, sizeof(szPort));
	inGetHostPortNoPrimary(szPort);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "CMAS_Port", BAD_CAST szPort);

	/* TMLocationID */
	memset(szMerchantID, 0x00, sizeof(szMerchantID));
	inGetMerchantID(szMerchantID);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "TMLocationID", BAD_CAST szMerchantID);

	/* TMID */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "TMID", BAD_CAST "01");

	/* TMAgentNumber */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "TMAgentNumber", BAD_CAST "0001");

	/* LocationID */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "LocationID", BAD_CAST "0");

	/* NewLocationID */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "NewLocationID", BAD_CAST "0");

	/* SPID */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "SPID", BAD_CAST "0");

	/* NewSPID */
	// memset(szMerchantID, 0x00, sizeof(szMerchantID));
	// inGetMerchantID(szMerchantID);
	// xmlNewChild(srRoot_Node, NULL, BAD_CAST "NewSPID", BAD_CAST szMerchantID);
	/* 20201215  test*/
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "NewSPID", BAD_CAST "00000000");

	/* Slot */
	memset(szSAMSlot, 0x00, sizeof(szSAMSlot));
	inGetTicket_SAM_Slot(szSAMSlot);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (memcmp(szSAMSlot, "01", strlen("01")) == 0)
	{
		sprintf(szTemplate, "11");
	}
	else if (memcmp(szSAMSlot, "02", strlen("02")) == 0)
	{
		sprintf(szTemplate, "22");
	}
	else if (memcmp(szSAMSlot, "03", strlen("03")) == 0)
	{
		sprintf(szTemplate, "33");
	}
	else if (memcmp(szSAMSlot, "04", strlen("04")) == 0)
	{
		sprintf(szTemplate, "44");
	}
	else
	{
		sprintf(szTemplate, "22"); /*悠遊卡預設Slot2*/
	}
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "Slot", BAD_CAST szTemplate);

	/* BaudRate */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "BaudRate", BAD_CAST "115200");

	/* OpenCom */
	/* QQ 問過儒勳，因為都是內建，所以OpenCom設什麼值都不影響*/
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "OpenCom", BAD_CAST "1");

	/* MustSettleDate */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "MustSettleDate", BAD_CAST "10");

	/* ReaderMode */
	/* QQ 2020/2/5 下午 2:35 這邊浩瑋和儒勳代的值不一樣，詳情要問悠遊卡 by Hachi*/
	/* 2020/2/27 下午 5:06 儒勳說當初是問家昌帶的值，目前帶的值和儒勳一樣*/
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ReaderMode", BAD_CAST "2");

	/* BatchFlag(批次檔案參數):0=EZHOST / 1=CMAS / 2=SIS2 */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "BatchFlag", BAD_CAST "1");

	/* OnlineFlag */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "OnlineFlag", BAD_CAST "1");

	/* ICERDataFlag */
	/*0 = 不產生ICERData資料夾 ， 1 = 產生ICERData資料夾 by Hachi*/
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ICERDataFlag", BAD_CAST "1");

	/* MessageHeader */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "MessageHeader", BAD_CAST "99909020");

	/* DLLMode */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "DLLMode", BAD_CAST "0");

	/* AutoLoadMode */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "AutoLoadMode", BAD_CAST "2");

	/* MaxALAmt */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "MaxALAmt", BAD_CAST "1500");/*最大加值金額由1000>1500*/

	/* Dev_Info */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "Dev_Info", BAD_CAST "1122334455");

	/* TCPIP_SSL */
	 memset(szTemplate, 0x00, sizeof(szTemplate));
	 memset(szIFESMode, 0x00, sizeof(szIFESMode));
	 inGetI_FES_Mode(szIFESMode);
	 if (memcmp(szIFESMode, "Y", strlen("Y")) == 0 )
	 {
	 	strcpy(szTemplate, "1");
	 }
	 else
	 {
	 	strcpy(szTemplate, "0");
	 }
	 xmlNewChild(srRoot_Node, NULL, BAD_CAST "TCPIP_SSL", BAD_CAST szTemplate);
	//xmlNewChild(srRoot_Node, NULL, BAD_CAST "TCPIP_SSL", BAD_CAST "1"); /*20200519 test*/

	/* CMASAdviceVerify */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "CMASAdviceVerify", BAD_CAST "0");

	/* AutoSignOnPercnet */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "AutoSignOnPercnet", BAD_CAST "0");

	/* AutoLoadFunction */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "AutoLoadFunction", BAD_CAST "0");

	/* VerificationCode */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "VerificationCode", BAD_CAST "0");

	/* ReSendReaderAVR */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ReSendReaderAVR", BAD_CAST "0");

	/* XMLHeaderFlag */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "XMLHeaderFlag", BAD_CAST "1");

	/* FolderCreatFlag */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "FolderCreatFlag", BAD_CAST "1");

	/* <BLCName> : 黑名單 */
	// xmlNewChild(srRoot_Node, NULL, BAD_CAST "BLCName", BAD_CAST "BLC00762A_170413.BIG");
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "BLCName", BAD_CAST "BLC01095BIG");

	/* CMASMode */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "CMASMode", BAD_CAST "1");

	/* POS_ID */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "POS_ID", BAD_CAST "0");

	/* PacketLenFlag */
	/*直連悠遊卡要0,vpn要1*/ /*玉山都是1*/ /*PacketLenFlag:電文長度旗標，0=電文沒有長度 / 1=電文有長度*/
	/* 2020/2/21 下午 4:31 這邊指的是如果是VPN，電文需要多帶長度[Hachi]*/
	
	/* 20200413[Hachi] start*/
	if(memcmp(szIFESMode, "Y", strlen("Y")) == 0 )
	{
		xmlNewChild(srRoot_Node, NULL, BAD_CAST "PacketLenFlag", BAD_CAST "0");
	}
	else
	{
		xmlNewChild(srRoot_Node, NULL, BAD_CAST "PacketLenFlag", BAD_CAST "1");
	}
	/* 20200413[Hachi] end*/

	/*For 玉山測試專用*/
	// xmlNewChild(srRoot_Node, NULL, BAD_CAST "PacketLenFlag", BAD_CAST "1");

	/*  
        AdditionalTcpipData:附加額外的TCPIP資料(有額外需附加的資料時,請參照2.1.6章節)
        0=沒有額外需附加的資料
        1=有額外需附加的資料(NCCC)
        2=有額外需附加的資料(CTCB)
        3=有額外需附加的資料(FISC)
	*/
	/* 20200406 目前測試SAM卡合庫走財金 AdditionalTcpipData= 3*/
	/* 20200406 目前玉山偽直連 AdditionalTcpipData = 0*/
	memset(szCMASAddTcpipMode, 0x00, sizeof(szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);	

	if (szCMASAddTcpipMode[0] == '0')
		xmlNewChild(srRoot_Node, NULL, BAD_CAST "AdditionalTcpipData", BAD_CAST "0");
	else if (szCMASAddTcpipMode[0] == '1')
	 	xmlNewChild(srRoot_Node, NULL, BAD_CAST "AdditionalTcpipData", BAD_CAST "1");
	else if (szCMASAddTcpipMode[0] == '2')
		xmlNewChild(srRoot_Node, NULL, BAD_CAST "AdditionalTcpipData", BAD_CAST "2");	
	else if (szCMASAddTcpipMode[0] == '3')
		xmlNewChild(srRoot_Node, NULL, BAD_CAST "AdditionalTcpipData", BAD_CAST "3");
	else
		xmlNewChild(srRoot_Node, NULL, BAD_CAST "AdditionalTcpipData", BAD_CAST "3");	
	
	/* CRT_FileName */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "CRT_FileName", BAD_CAST " ");
	/*For 玉山測試專用*/
	// xmlNewChild(srRoot_Node, NULL, BAD_CAST "CRT_FileName", BAD_CAST "npotest.esunbank.com.tw.pem");

	/* Key_FileName */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "Key_FileName", BAD_CAST " ");

	/* ICERFlowDebug 關閉 = 0; 打開 = 1 */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ICERFlowDebug", BAD_CAST "1");

	/* ReaderUartDebug 悠遊卡測FW使用 */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ReaderUartDebug", BAD_CAST "0");

	/* 儲存xml檔 xml檔現在會自動換行了 */
	inXML_SaveFile(_CMAS_PARA_FILE_, &srDoc, "utf-8", 1);

	/* 清空佔用記憶體 */
	inXML_End(&srDoc);

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PACK_Deduct_ICERAPI
Date&Time       :2020/2/6 下午 3:40
Describe        :組購貨XML
Author          :Hachi
*/
int inCMAS_PACK_Deduct_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	int inESVCIndex = 0;
	// int inHostSeriNO = 0;
	char szTemplate[20 + 1] = {0};
	char szSTAN[6 + 1] = {0};
	char szInvoiceNum[6 + 1] = {0};
	char szCMASAddTcpipMode[ 2 + 1 ] ={0};
	unsigned long ulHandle = 0;
	RTC_NEXSYS srRTC = {0};
	xmlDocPtr srDoc = NULL;
	xmlNodePtr srRoot_Node = NULL, srTRANS_Node = NULL;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_Deduct_ICERAPI() START !");
	}

	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
		return (VS_ERROR);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);

	if (inLoadEDCRec(0) != VS_SUCCESS)
		return (VS_ERROR);
	if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);

	/* 刪除原本的ICERAPI */
	inFile_Unlink_File(_CMAS_API_REQ_CHECK_, _CMAS_FOLDER_PATH_); /* 檢核用 */
	inFile_Unlink_File(_CMAS_API_REQ_FILE_, _CMAS_FOLDER_PATH_);
	inFile_Unlink_File(_CMAS_API_REQ_FILE2_, _CMAS_FOLDER_PATH_);
	inFile_Unlink_File(_CMAS_API_RES_FILE_, _CMAS_FOLDER_PATH_); /* 防止用到上次資料結果 */
	/* Retry時需要參考RES2，不能砍 */
	if (guszECCRetryBit == VS_TRUE)
	{
	}
	else
	{
		inFile_Unlink_File(_CMAS_API_RES_FILE2_, _CMAS_FOLDER_PATH_);
	}

	/* 建立一個新的檔案 */
	srDoc = xmlNewDoc(BAD_CAST "1.0");
	srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
	xmlDocSetRootElement(srDoc, srRoot_Node);

	/* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

	/* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0200");

	/* <T0300> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "606100");

	/* <T0400> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%010lu00", pobTran->srTRec.lnTxnAmount);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0400", BAD_CAST szTemplate);

	memset(szCMASAddTcpipMode, 0x00, sizeof(szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);	
	
	if (guszECCRetryBit == VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT,"in %s(line:%d),guszECCRetryBit == VS_TRUE", __FUNCTION__, __LINE__);
			inDISP_LogPrintfAt(AT,"szT1100 = %s", gszECCRetryData.szT1100);
			inDISP_LogPrintfAt(AT,"szT1101 = %s", gszECCRetryData.szT1101);
			inDISP_LogPrintfAt(AT,"szT1102 = %s", gszECCRetryData.szT1102);
			inDISP_LogPrintfAt(AT,"szT1103 = %s", gszECCRetryData.szT1103);
		}
		/* Retry 要同第一次，除了時間 */
		/* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST gszECCRetryData.szT1100);

		/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
		/* 20201216 帶T1101測試 */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST gszECCRetryData.szT1101);

		/* 20200416 AdditionalTcpipData != 0 時 <T1102>，<T1103>需要帶[Hachi]*/	
		if (szCMASAddTcpipMode[0] != '0')
		{
			/* <T1102> : NCCC STAN */
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST gszECCRetryData.szT1102);

			/* <T1103> : NCCC RRN(後三碼) */
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST gszECCRetryData.szT1103);
		}
		/* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT,"in %s(line:%d),guszECCRetryBit == VS_FALSE", __FUNCTION__, __LINE__);
			inDISP_LogPrintfAt(AT,"szT1100 = %s", gszECCRetryData.szT1100);
			inDISP_LogPrintfAt(AT,"szT1101 = %s", gszECCRetryData.szT1101);
			inDISP_LogPrintfAt(AT,"szT1102 = %s", gszECCRetryData.szT1102);
			inDISP_LogPrintfAt(AT,"szT1103 = %s", gszECCRetryData.szT1103);
		}
		/* For Retry 重試時使用 */
		memset(&gszECCRetryData, 0x00, sizeof(gszECCRetryData));

		/* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN); //2020.03.12.mark by Hachi 

		/* 2020.03.12.*/
		// memset(szTemplate, 0x00, sizeof(szTemplate));
		// sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
		// xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szTemplate);

		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1100, szSTAN, strlen(szSTAN)); //2020.03.12 mark by Hachi
		// memcpy(gszECCRetryData.szT1100, szSTAN, strlen(szTemplate));

		/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetTicket_STAN(szSTAN);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
		inSetTicket_STAN(szTemplate);
		inSaveTDTRec(_TDT_INDEX_01_ECC_);

		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1101, szTemplate, strlen(szTemplate));

		/* 20200416AdditionalTcpipData != 0 時 <T1102>，<T1103>需要帶[Hachi] */
		if (szCMASAddTcpipMode[0] != '0')
		{
			/* <T1102> : NCCC STAN */
			memset(szSTAN, 0x00, sizeof(szSTAN));
			inGetSTANNum(szSTAN);
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);

			/* For Retry 重試時使用 */
			memcpy(gszECCRetryData.szT1102, szSTAN, strlen(szSTAN));

			/* <T1103> : NCCC RRN(後三碼) */
			memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
			inGetInvoiceNum(szInvoiceNum);
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);

			/* For Retry 重試時使用 */
			memcpy(gszECCRetryData.szT1103, szInvoiceNum, strlen(szInvoiceNum));
		}
		/* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}

	/* <T4108> : LCD Control Flag 單純顯示畫面用 */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4108", BAD_CAST "0");

	/* <T4830> : Read Purse Flag 
			0：不需同一票卡(default)
			1：需為同一票卡
	*/
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4830", BAD_CAST "0");

	/* <T5501> : Batch Number - 悠遊卡特有，同一天流水號累加，隔天就要歸1 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inCMAS_Batch_Check(szTemplate);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5501", BAD_CAST szTemplate);
    
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"in %s(%d)，",__FUNCTION__,__LINE__);
		inDISP_LogPrintfAt(AT,"T5501 Batch Number = %s", szTemplate);
	}

	/* 儲存xml檔 xml檔現在會自動換行了 */
	inXML_SaveFile(_CMAS_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_CMAS_API_REQ_FILE_, _AP_ROOT_PATH_, "", _CMAS_FOLDER_PATH_);

	/* 清空佔用記憶體 */
	inXML_End(&srDoc);

	/* 步驟完成，建立檢核擋 */
	inRetVal = inFILE_Create(&ulHandle, (unsigned char *)_CMAS_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_CMAS_API_REQ_CHECK_, _FS_DATA_PATH_, "", _CMAS_FOLDER_PATH_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_Deduct_ICERAPI() END !");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_UNPACK_ICERAPI
Date&Time       :2020/2/11 下午 2:20
Describe        :從NCCC令雄版 inECC_UNPACK_ICERAPI 移植過來
Author          :Hachi
*/
int inCMAS_UNPACK_ICERAPI(TRANSACTION_OBJECT *pobTran, char *szFileName)
{
	int i = 0;
	int inRetVal = VS_ERROR;
	int inESVCIndex = -1;
	int inOffset = 0;
	char szTemplate[20 + 1] = {0};
	char szTagData[1536 + 1] = {0};
	char szDebugMsg[100 + 1];
	char szLastTransDate[8 + 1] = {0};
	char szLastTransRRN[20 + 1] = {0};
	char szT1100[6 + 1] = {0};
	char szT1101[6 + 1] = {0};
	char szT1102[6 + 1] = {0};
	char szT4100[20 + 1] = {0};
	char szT4109[20 + 1] = {0};
	char szT4110[20 + 1] = {0};
	xmlChar *szTagPtr = NULL;
	xmlDocPtr srDoc = NULL;
	xmlNodePtr srCur = NULL;	  /* (第二層) */
	xmlNodePtr srTagNode = NULL;  /* 用來移動Tag那一層(第三層) */
	xmlNodePtr srTextNode = NULL; /* 因為ElementNode的子節點TextNode才有內容(第四層) */

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inCMAS_UNPACK_ICERAPI() START !");
	}
	
	
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);
	inLoadHDTRec(inESVCIndex);
	inLoadHDPTRec(inESVCIndex);

	inRetVal = inECC_ParseXML_In_ICERData(szFileName, &srDoc);
	/* 異常 */
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
 
	inRetVal = inXML_Get_RootElement(&srDoc, &srCur);
	/* 異常 */
	if (inRetVal != VS_SUCCESS)
	{
		inXML_End(&srDoc);

		return (VS_ERROR);
	}

	/* 先走到TransXML的下一層 */
	while (srCur->children != NULL)
	{
		srCur = srCur->children;
		if (memcmp(srCur->parent->name, "TransXML", strlen("TransXML")) == 0)
		{
			break;
		}
	}

	memset(szT1100, 0x00, sizeof(szT1100));
	memset(szT1101, 0x00, sizeof(szT1101));
	memset(szT1102, 0x00, sizeof(szT1102));
	memset(szT4100, 0x00, sizeof(szT4100));
	memset(szT4109, 0x00, sizeof(szT4109));
	memset(szT4110, 0x00, sizeof(szT4110));

	/* 換下一個節點檢查，往下走到TRANS的子節點，接著開始往橫走 */
	while (srCur != NULL)
	{
		/* 每次都設為NULL，如果有要查看的話才設定位置 */
		srTagNode = NULL;
		if (memcmp(srCur->name, "TRANS", strlen("TRANS")) == 0 &&
			srCur->children != NULL)
		{
			srTagNode = srCur->children;
		}

		/* 推移第三層 */
		while (srTagNode != NULL)
		{
			/* 有分XML_TEXT_NODE和XML_ELEMENT_NODE 才有content可抓 */
			if (srTagNode->type == XML_ELEMENT_NODE &&
				(srTagNode->children != NULL &&
				 srTagNode->children->type == XML_TEXT_NODE))
			{
				srTextNode = srTagNode->children;
				memset(szTagData, 0x00, sizeof(szTagData));
				szTagPtr = xmlNodeListGetString(srDoc, srTextNode, srTextNode->line);
				if (szTagPtr != NULL &&
					xmlStrlen(szTagPtr) != 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s : %s", srTagNode->name, szTagPtr);
						inDISP_LogPrintfAt(AT,szDebugMsg);
					}

					strcat(szTagData, (char *)szTagPtr);
				}
			}

			/* 悠遊卡號內碼(改列印這個) */
			if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T0200") == 0)
			{
                            /*20211130,悠遊卡告知用T0215*/
//				inOffset = 0;
//				for (i = 0; i < strlen(szTagData); i++)
//				{
//					/* 非0位置 */
//					if (szTagData[i] != '0')
//					{
//						inOffset = i;
//						break;
//					}
//				}
//
//				/* 悠遊卡號內碼 */
//				memset(pobTran->srTRec.szUID, 0x00, sizeof(pobTran->srTRec.szUID));
//				memcpy(&pobTran->srTRec.szUID[0], &szTagData[inOffset], (strlen(szTagData) - inOffset));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T0215") == 0)
			{
				inOffset = 0;
				for (i = 0; i < strlen(szTagData); i++)
				{
					/* 非0位置 */
					if (szTagData[i] != '0')
					{
						inOffset = i;
						break;
					}
				}

				/* 悠遊卡號內碼 */
				memset(pobTran->srTRec.szUID, 0x00, sizeof(pobTran->srTRec.szUID));
				memcpy(&pobTran->srTRec.szUID[0], &szTagData[inOffset], (strlen(szTagData) - inOffset));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T0400") == 0)
			{
				/* 金額 */
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T0409") == 0)
			{
				/* 自動加值金額 */
				if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &szTagData[0], (strlen(szTagData) - 2));
					pobTran->srTRec.lnTotalTopUpAmount = atol(szTemplate);
                			/*20210426 mark[Hachi]*/
                                        //pobTran->srTRec.uszAutoTopUpBit = VS_TRUE; /* 20200515[Hachi]*/  
				}
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T0410") == 0)
			{
				/* 交易後餘額 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szTagData[0], (strlen(szTagData) - 2));
				pobTran->srTRec.lnFinalAfterAmt = atol(szTemplate);
				pobTran->srTRec.lnCardRemainAmount = pobTran->srTRec.lnFinalAfterAmt;
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T0415") == 0)
			{
				/* 交易前餘額 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &szTagData[0], (strlen(szTagData) - 2));
				pobTran->srTRec.lnFinalBeforeAmt = atol(szTemplate);
			}
			/* 20200618 Add T1100 Hachi*/
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T1100") == 0)
			{
				/* 端末設備交易序號 */
				if (atol(szTagData) != 0)
				{
					sprintf(szT1101, "%06ld", atol(szTagData));
				}
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T1101") == 0)
			{
				/* Host Serial Number */
				if (atol(szTagData) != 0)
				{
					sprintf(szT1101, "%06ld", atol(szTagData));
				}
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T1102") == 0)
			{
				/* NCCC STAN */
				if (atol(szTagData) != 0)
				{
					sprintf(szT1102, "%06ld", atol(szTagData));
				}
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T1103") == 0)
			{
				/* NCCC RRN */
				if (atol(szTagData) != 0)
				{
					/* 悠遊卡會累加成下一筆 */
					pobTran->srTRec.lnECCInvNum = atol(szTagData);
				}
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T1200") == 0)
			{
				/* 交易時間 HHMMSS */
				memset(pobTran->srTRec.srECCRec.szTime, 0x00, sizeof(pobTran->srTRec.srECCRec.szTime));
				memcpy(&pobTran->srTRec.srECCRec.szTime[0], &szTagData[0], 6);
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T1300") == 0)
			{
				/* 交易日期 YYYYMMDD */
				memset(pobTran->srTRec.srECCRec.szDate, 0x00, sizeof(pobTran->srTRec.srECCRec.szDate));
				memcpy(&pobTran->srTRec.srECCRec.szDate[0], &szTagData[0], 8);

				memset(szLastTransDate, 0x00, sizeof(szLastTransDate));
				memcpy(szLastTransDate, &szTagData[0], 8);
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T3700") == 0)
			{
				/* RRN */
				memset(pobTran->srTRec.srECCRec.szRRN, 0x00, sizeof(pobTran->srTRec.srECCRec.szRRN));
				memcpy(&pobTran->srTRec.srECCRec.szRRN[0], &szTagData[0], strlen(szTagData));

				memset(szLastTransRRN, 0x00, sizeof(szLastTransRRN));
				memcpy(szLastTransRRN, &szTagData[0], strlen(szTagData));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T3800") == 0)
			{
				/* AuthCode */
				memset(pobTran->srTRec.szAuthCode, 0x00, sizeof(pobTran->srTRec.szAuthCode));
				memcpy(&pobTran->srTRec.szAuthCode[0], &szTagData[0], strlen(szTagData));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T3900") == 0)
			{
				/* ICER RespCode */
				memset(pobTran->srTRec.srECCRec.szICER_RespCode, 0x00, sizeof(pobTran->srTRec.srECCRec.szICER_RespCode));
				memcpy(&pobTran->srTRec.srECCRec.szICER_RespCode[0], &szTagData[0], strlen(szTagData));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T3901") == 0)
			{
				/* API RespCode */
				memset(pobTran->srTRec.srECCRec.szAPI_RespCode, 0x00, sizeof(pobTran->srTRec.srECCRec.szAPI_RespCode));
				memcpy(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], &szTagData[0], strlen(szTagData));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T3902") == 0)
			{
				/* EZHOST RespCode */
				memset(pobTran->srTRec.srECCRec.szEZ_RespCode, 0x00, sizeof(pobTran->srTRec.srECCRec.szEZ_RespCode));
				memcpy(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], &szTagData[0], strlen(szTagData));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T3903") == 0)
			{
				/* SVCS RespCode */
				memset(pobTran->srTRec.srECCRec.szSVCS_RespCode, 0x00, sizeof(pobTran->srTRec.srECCRec.szSVCS_RespCode));
				memcpy(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], &szTagData[0], strlen(szTagData));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T3904") == 0)
			{
				/* Reader RespCode */
				memset(pobTran->srTRec.srECCRec.szReader_RespCode, 0x00, sizeof(pobTran->srTRec.srECCRec.szReader_RespCode));
				memcpy(&pobTran->srTRec.srECCRec.szReader_RespCode[0], &szTagData[0], strlen(szTagData));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T3908") == 0)
			{
				/* NCCC Host RespCode */
				memset(pobTran->srTRec.szRespCode, 0x00, sizeof(pobTran->srTRec.szRespCode));
				memcpy(&pobTran->srTRec.szRespCode[0], &szTagData[0], strlen(szTagData));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T4109") == 0)
			{
				/* 一代設備編號 */
				strcpy(szT4109, szTagData);
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T4100") == 0)
			{
				/* T5581使用 */
				strcpy(szT4100, szTagData);
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T4110") == 0)
			{
				/* 二代設備編號 */
				strcpy(szT4110, szTagData);
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T3912") == 0)
			{
				/* 悠遊卡結帳狀態 */
				/* [新增電票悠遊卡功能] 新增悠遊卡結帳狀態，但不需存在批次中，所以只存在每筆交易記錄 [SAM] 2022/6/30 下午 3:59 */
				pobTran->szTicketSettleBlance[0] = szTagData[0];
			}else if (xmlStrcmp(srTagNode->name, (const xmlChar *)"T4803") == 0)
			{
				/* Bank Code */
				/* [新增電票悠遊卡功能] 新增悠遊卡Bank Code，但不需存在批次中，所以只存在每筆交易記錄 [SAM] 2022/6/30 下午 3:59 */
				pobTran->szTicketBankId[0] = szTagData[0];
			}

			srTagNode = srTagNode->next;
		}

		/* 推移第二層 */
		srCur = srCur->next;
	}

	/* free */
	inXML_End(&srDoc);

	/* 20200618 Added T1101[Hahci]*/
	if (strlen(szT1100) > 0)
	{
		inSetSTANNum(szT1100);
	}
	if (strlen(szT1101) > 0)
	{
		inSetTicket_STAN(szT1101);
	}
	if (strlen(szT1102) > 0)
	{
		inSetSTANNum(szT1102);
	}
	if (strlen(szT4109) > 0)
	{
		inSetTicket_Device1(szT4109);
	}
	if (strlen(szT4100) > 0)
	{
		inSetTicket_Device3(szT4100);
	}
	if (strlen(szT4110) > 0)
	{
		inSetTicket_Device2(szT4110);
	}

	/* 邊解邊存的話，有可能這筆購貨失敗但上傳鎖卡advice，此時會存到鎖卡的RRN和Lastdate，導致取消加值帶錯資料 */
	if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) == 0)
	{

		if (strlen(szLastTransDate) > 0)
		{
			if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ ||
				pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_ ||
				pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
				pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
				pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
			{
				/* 存最後一筆交易的時間 */
				inSetTicket_LastTransDate(szLastTransDate);
			}
		}

		if (strlen(szLastTransRRN) > 0)
		{
			inSetTicket_LastRRN(szLastTransRRN);
		}
	}

	// inSaveHDPTRec(inESVCIndex);  //200618  mark
	inSaveTDTRec(_TDT_INDEX_01_ECC_);

	/*20200618 累加STAN*/
	inCMAS_ISO_SetSTAN(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_UNPACK_ICERAPI() END !");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PACK_Refund_ICERAPI
Date&Time       :2020/2/11 下午 5:36
Describe        :組退貨XML
Author          :Hachi
*/
int inCMAS_PACK_Refund_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	int inESVCIndex = 0;
	char szTemplate[20 + 1] = {0};
	char szSTAN[6 + 1] = {0};
	char szInvoiceNum[6 + 1] = {0};
	char szCMASAddTcpipMode[ 2 + 1 ] ={0};
	unsigned long ulHandle = 0;
	RTC_NEXSYS srRTC = {0};
	xmlDocPtr srDoc = NULL;
	xmlNodePtr srRoot_Node = NULL, srTRANS_Node = NULL;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_Refund_ICERAPI() START !");
	}

	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
		return (VS_ERROR);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);

	if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);

	/* 刪除原本的ICERAPI */
	inFILE_Delete((unsigned char *)_ECC_API_REQ_CHECK_); /* 檢核用 */
	inFILE_Delete((unsigned char *)_ECC_API_REQ_FILE_);
	inFILE_Delete((unsigned char *)_ECC_API_REQ_FILE2_);
	inFILE_Delete((unsigned char *)_ECC_API_RES_FILE_); /* 防止用到上次資料結果 */
	inFILE_Delete((unsigned char *)_ECC_API_RES_FILE2_);

	/* 建立一個新的檔案 */
	srDoc = xmlNewDoc(BAD_CAST "1.0");
	srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
	xmlDocSetRootElement(srDoc, srRoot_Node);

	/* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

	/* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0200");

	/* <T0300> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "620061");

	/* <T0400> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%010lu00", pobTran->srTRec.lnTxnAmount);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0400", BAD_CAST szTemplate);

	memset(szCMASAddTcpipMode, 0x00, sizeof(szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);

	if (guszECCRetryBit == VS_TRUE)
	{
		/* Retry 要同第一次，除了時間 */
		/* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST gszECCRetryData.szT1100);

		/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
		/* 20201216 帶T1101測試 */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST gszECCRetryData.szT1101);

		/*  20200416 AdditionalTcpipData != 0 時 <T1102>，<T1103>需要帶[Hachi] end */
		if (szCMASAddTcpipMode[0] != '0')
		{
			/* <T1102> : NCCC STAN */
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST gszECCRetryData.szT1102);

			/* <T1103> : NCCC RRN(後三碼) */
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST gszECCRetryData.szT1103);
		}
		/* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}
	else
	{
		/* For Retry 重試時使用 */
		memset(&gszECCRetryData, 0x00, sizeof(gszECCRetryData));

		/* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);

		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1100, szSTAN, strlen(szSTAN));

		/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetTicket_STAN(szSTAN);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
		inSetTicket_STAN(szTemplate);
		inSaveTDTRec(_TDT_INDEX_01_ECC_);

		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1101, szTemplate, strlen(szTemplate));


		/*  20200416 AdditionalTcpipData != 0 時 <T1102>，<T1103>需要帶[Hachi] start */
		if (szCMASAddTcpipMode[0] != '0')
		{
			/* <T1102> : NCCC STAN */
			memset(szSTAN, 0x00, sizeof(szSTAN));
			inGetSTANNum(szSTAN);
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);

			// /* For Retry 重試時使用 */
			memcpy(gszECCRetryData.szT1102, szSTAN, strlen(szSTAN));

			// /* <T1103> : NCCC RRN(後三碼) */
			memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
			inGetInvoiceNum(szInvoiceNum);
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);

			// /* For Retry 重試時使用 */
			memcpy(gszECCRetryData.szT1103, szInvoiceNum, strlen(szInvoiceNum));
		}

		/* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}

	/* <T4108> : LCD Control Flag 單純顯示畫面用 */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4108", BAD_CAST "0");

	/* <T4830> : Read Purse Flag 
		0：不需同一票卡(default)
		1：需為同一票卡
	*/
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4830", BAD_CAST "0");

	/* <T5501> : Batch Number - 悠遊卡特有，同一天流水號累加，隔天就要歸1 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inCMAS_Batch_Check(szTemplate);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5501", BAD_CAST szTemplate);

	/* 儲存xml檔 xml檔現在會自動換行了 */
	inXML_SaveFile(_ECC_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);

	/* 清空佔用記憶體 */
	inXML_End(&srDoc);

	/* 步驟完成，建立檢核擋 */
	inRetVal = inFILE_Create(&ulHandle, (unsigned char *)_ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	inFile_Move_File(_ECC_API_REQ_FILE_, _FS_DATA_PATH_, "", _ECC_FOLDER_PATH_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_Refund_ICERAPI() END !");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PACK_Inquiry_ICERAPI
Date&Time       :2020/2/11 下午 6:24
Describe        :組餘額查詢XML
Author          :Hachi
*/
int inCMAS_PACK_Inquiry_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	int inESVCIndex = 0;
	char szTemplate[20 + 1] = {0};
	char szSTAN[6 + 1] = {0};
	char szInvoiceNum[6 + 1] = {0};
	char szCMASAddTcpipMode[ 2 + 1 ] ={0};
	unsigned long ulHandle = 0;
	RTC_NEXSYS srRTC = {0};
	xmlDocPtr srDoc = NULL;
	xmlNodePtr srRoot_Node = NULL, srTRANS_Node = NULL;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_Inquiry_ICERAPI() START !");
	}

	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
		return (VS_ERROR);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);

	if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);

	// if (inLoadECCDTRec(0) != VS_SUCCESS)
	// {
	// 	return (VS_ERROR);
	// }

	/* 刪除原本的ICERAPI */
	inFile_Unlink_File(_CMAS_API_REQ_CHECK_, _CMAS_FOLDER_PATH_); /* 檢核用 */
	inFile_Unlink_File(_CMAS_API_REQ_FILE_, _CMAS_FOLDER_PATH_);
	inFile_Unlink_File(_CMAS_API_REQ_FILE2_, _CMAS_FOLDER_PATH_);
	inFile_Unlink_File(_CMAS_API_RES_FILE_, _CMAS_FOLDER_PATH_); /* 防止用到上次資料結果 */
	/* Retry時需要參考RES2，不能砍 */
	if (guszECCRetryBit == VS_TRUE)
	{
	}
	else
	{
		inFile_Unlink_File(_CMAS_API_RES_FILE2_, _CMAS_FOLDER_PATH_);
	}

	/* 建立一個新的檔案 */
	srDoc = xmlNewDoc(BAD_CAST "1.0");
	srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
	xmlDocSetRootElement(srDoc, srRoot_Node);

	/* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

	/* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0200");

	/* <T0300> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "296000");

	/* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);

	/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetTicket_STAN(szSTAN);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
	inSetTicket_STAN(szTemplate);
	inSaveTDTRec(_TDT_INDEX_01_ECC_);

	memset(szCMASAddTcpipMode, 0x00, sizeof(szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);	

	/*  2020.03.19 AdditionalTcpipData != 0 時 <T1102>，<T1103>需要帶[Hachi] */
	if (szCMASAddTcpipMode[0] != '0')
	{
		/* <T1102> : NCCC STAN */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);

		/* <T1103> : NCCC RRN(後三碼) */
		memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
		inGetInvoiceNum(szInvoiceNum);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);

	}
	/* <T1200> : hhmmss */
	memset(&srRTC, 0x00, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

	/* <T1300> : yyyymmdd */
	/* hhmmss就抓過一次時間了，不用重抓 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);

	/* <T4108> : LCD Control Flag 單純顯示畫面用 */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4108", BAD_CAST "0");

	/* 儲存xml檔 xml檔現在會自動換行了 */
	inXML_SaveFile(_CMAS_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_CMAS_API_REQ_FILE_, _AP_ROOT_PATH_, "", _CMAS_FOLDER_PATH_);

	/* 清空佔用記憶體 */
	inXML_End(&srDoc);

	/* 步驟完成，建立檢核擋 */
	inRetVal = inFILE_Create(&ulHandle, (unsigned char *)_CMAS_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	inFile_Move_File(_CMAS_API_REQ_CHECK_, _FS_DATA_PATH_, "", _CMAS_FOLDER_PATH_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_Inquiry_ICERAPI() END !");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PACK_TOP_UP_ICERAPI
Date&Time       :2020/2/18 下午 2:24
Describe        :組現金加值XML
Author          :Hachi
*/
int inCMAS_PACK_TOP_UP_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	int inESVCIndex = -1;
	char szSTAN[12 + 1] = {0};
	char szInvoiceNum[6 + 1] = {0};
	char szTemplate[20 + 1] = {0};
	char szCMASAddTcpipMode[ 2 + 1 ] ={0};
	unsigned long ulHandle = 0;
	RTC_NEXSYS srRTC = {0};
	xmlDocPtr srDoc = NULL;
	xmlNodePtr srRoot_Node = NULL, srTRANS_Node = NULL;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_TOP_UP_ICERAPI() START !");
	}

	inFunc_Find_Specific_HDTindex(-1, _HOST_NAME_CMAS_, &inESVCIndex);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inESVCIndex (%d)", inESVCIndex);
	}

	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
		return (VS_ERROR);
	if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);

	// /*取出STAN*/
	// inGetSTANNum(szSTAN);

	// /*取出InvNum*/
	// inGetTicket_InvNum(szInvoiceNum);

	/* 刪除原本的ICERAPI */
	inFile_Unlink_File(_CMAS_API_REQ_CHECK_, _CMAS_FOLDER_PATH_); /* 檢核用 */
	inFile_Unlink_File(_CMAS_API_REQ_FILE_, _CMAS_FOLDER_PATH_);
	inFile_Unlink_File(_CMAS_API_REQ_FILE2_, _CMAS_FOLDER_PATH_);
	inFile_Unlink_File(_CMAS_API_RES_FILE_, _CMAS_FOLDER_PATH_); /* 防止用到上次資料結果 */
	/* Retry時需要參考RES2，不能砍 */
	if (guszECCRetryBit == VS_TRUE)
	{
	}
	else
	{
		inFile_Unlink_File(_CMAS_API_RES_FILE2_, _CMAS_FOLDER_PATH_);
	}	


	/* 建立一個新的檔案 */
	srDoc = xmlNewDoc(BAD_CAST "1.0");
	srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
	xmlDocSetRootElement(srDoc, srRoot_Node);

	/* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

	/* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0200");

	/* <T0300> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "801061");

	/* <T0400> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%010lu00", pobTran->srTRec.lnTxnAmount);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0400", BAD_CAST szTemplate);

	memset(szCMASAddTcpipMode, 0x00, sizeof(szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);
	
	if (guszECCRetryBit == VS_TRUE)
	{
		/* Retry 要同第一次，除了時間 */
		/* <T1100> : 端末設備交易序號(先塞STAN) */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST gszECCRetryData.szT1100);

		/* <T1101> : HOST SERIAL NUMBER 20200220 ask Green 說不用送[Hachi]*/
		/* 20201216 帶T1101測試 */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST gszECCRetryData.szT1101);

		/* 20200416 AdditionalTcpipData != 0 時 <T1102>，<T1103>需要帶[Hachi]*/	
		if (szCMASAddTcpipMode[0] != '0')
		{
			/* <T1102> : STAN */
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST gszECCRetryData.szT1102);

			/* <T1103> : Invoice Num */
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST gszECCRetryData.szT1103);
		}
		/* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}
	else
	{
		/* For Retry 重試時使用 */
		memset(&gszECCRetryData, 0x00, sizeof(gszECCRetryData));

		/* <T1100> : 端末設備交易序號(先塞STAN)  */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);

		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1100, szSTAN, strlen(szSTAN));

		/* <T1101> : HOST SERIAL NUMBER ，20200220 ask Green 說不用送[Hachi]*/
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetTicket_STAN(szSTAN);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
		inSetTicket_STAN(szTemplate);
		inSaveTDTRec(_TDT_INDEX_01_ECC_);

		// /* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1101, szTemplate, strlen(szTemplate));
		/* 20200416 AdditionalTcpipData != 0 時 <T1102>，<T1103>需要帶[Hachi]*/	
		if (szCMASAddTcpipMode[0] != '0')
		{
			/* <T1102> : STAN */
			memset(szSTAN, 0x00, sizeof(szSTAN));
			inGetSTANNum(szSTAN);
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);

			/* For Retry 重試時使用 */
			memcpy(gszECCRetryData.szT1102, szSTAN, strlen(szSTAN));

			/* <T1103> : Invoice Number */
			memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
			inGetInvoiceNum(szInvoiceNum);
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);

			/* For Retry 重試時使用 */
			memcpy(gszECCRetryData.szT1103, szInvoiceNum, strlen(szInvoiceNum));
		}
		/* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}

	/* <T4108> : LCD Control Flag 單純顯示畫面用 */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4108", BAD_CAST "0");

	/* <T4830> : Read Purse Flag 
	         0：不需同一票卡(default)
		     1：需為同一票卡
	*/
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4830", BAD_CAST "0");

	/* <T5501> : Batch Number - 悠遊卡特有，同一天流水號累加，隔天就要歸1 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inCMAS_Batch_Check(szTemplate);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5501", BAD_CAST szTemplate);

	/* 儲存xml檔 xml檔現在會自動換行了 */
	inXML_SaveFile(_CMAS_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_CMAS_API_REQ_FILE_, _AP_ROOT_PATH_, "", _CMAS_FOLDER_PATH_);

	/* 清空佔用記憶體 */
	inXML_End(&srDoc);

	/* 步驟完成，建立檢核擋 */
	inRetVal = inFILE_Create(&ulHandle, (unsigned char *)_CMAS_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	inFile_Move_File(_CMAS_API_REQ_CHECK_, _FS_DATA_PATH_, "", _CMAS_FOLDER_PATH_);
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_TOP_UP_ICERAPI() END !");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PACK_Void_TOP_UP_ICERAPI
Date&Time       :2020/2/20 下午 4:24
Describe        :組取消加值XML
Author          :Hachi
*/
int inCMAS_PACK_Void_TOP_UP_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	int inESVCIndex = -1;
	char szTemplate[20 + 1] = {0};
	char szSTAN[6 + 1] = {0};
	char szInvoiceNum[6 + 1] = {0};
	char szCMASAddTcpipMode[ 2 + 1 ] ={0};
	unsigned long ulHandle = 0;
	RTC_NEXSYS srRTC = {0};
	xmlDocPtr srDoc = NULL;
	xmlNodePtr srRoot_Node = NULL, srTRANS_Node = NULL;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_Void_TOP_UP_ICERAPI() START !");
	}

	inFunc_Find_Specific_HDTindex(-1, _HOST_NAME_CMAS_, &inESVCIndex);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inESVCIndex (%d)", inESVCIndex);
	}

	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
		return (VS_ERROR);
	if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);

	/* 刪除原本的ICERAPI */
	inFILE_Delete((unsigned char *)_CMAS_API_REQ_CHECK_); /* 檢核用 */
	inFILE_Delete((unsigned char *)_CMAS_API_REQ_FILE_);
	inFILE_Delete((unsigned char *)_CMAS_API_REQ_FILE2_);
	inFILE_Delete((unsigned char *)_CMAS_API_RES_FILE_); /* 防止用到上次資料結果 */
	inFILE_Delete((unsigned char *)_CMAS_API_RES_FILE2_);	

	/* 建立一個新的檔案 */
	srDoc = xmlNewDoc(BAD_CAST "1.0");
	srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
	xmlDocSetRootElement(srDoc, srRoot_Node);

	/* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

	/* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0200");

	/* <T0300> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "816100");

	/* <T0400> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%010lu00", pobTran->srTRec.lnTxnAmount);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0400", BAD_CAST szTemplate);

	memset(szCMASAddTcpipMode, 0x00, sizeof(szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);	
	
	if (guszECCRetryBit == VS_TRUE)
	{
		/* Retry 要同第一次，除了時間 */
		/* <T1100> : 端末設備交易序號(先塞STAN) */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST gszECCRetryData.szT1100);

		/* <T1101> : HOST SERIAL NUMBER 20200220 ask Green 說不用送[Hachi] */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST gszECCRetryData.szT1101);

		if (szCMASAddTcpipMode[0] != '0')
		{
			/* <T1102> : STAN */
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST gszECCRetryData.szT1102);

			/* <T1103> : Invoice Num */
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST gszECCRetryData.szT1103);
		}
		/* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}
	else
	{
		/* For Retry 重試時使用 */
		memset(&gszECCRetryData, 0x00, sizeof(gszECCRetryData));

		/* <T1100> : 端末設備交易序號(先塞STAN) */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);

		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1100, szSTAN, strlen(szSTAN));

		/* <T1101> : HOST SERIAL NUMBER 20200220 ask Green 說不用送[Hachi] */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetTicket_STAN(szSTAN);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
		inSetTicket_STAN(szTemplate);
		inSaveTDTRec(_TDT_INDEX_01_ECC_);

		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1101, szTemplate, strlen(szTemplate));

		/* 20200416 AdditionalTcpipData != 0 時 <T1102>，<T1103>需要帶[Hachi]*/	
		if (szCMASAddTcpipMode[0] != '0')
		{
			/* <T1102> : STAN */
			memset(szSTAN, 0x00, sizeof(szSTAN));
			inGetSTANNum(szSTAN);
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);

			/* For Retry 重試時使用 */
			memcpy(gszECCRetryData.szT1102, szSTAN, strlen(szSTAN));

			/* <T1103> : Invoice Num */
			memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
			inGetInvoiceNum(szInvoiceNum);
			xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);
			
			/* For Retry 重試時使用 */
			memcpy(gszECCRetryData.szT1103, szInvoiceNum, strlen(szInvoiceNum));
		}
		/* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}

	/* <T4108> : LCD Control Flag 單純顯示畫面用 */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4108", BAD_CAST "0");

	/* <T4830> : Read Purse Flag 
	             0：不需同一票卡(default)
		     1：需為同一票卡
	*/
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4830", BAD_CAST "0");

	/* <T5501> : Batch Number - 悠遊卡特有，同一天流水號累加，隔天就要歸1 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inCMAS_Batch_Check(szTemplate);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5501", BAD_CAST szTemplate);

	/* <T5581> : 取消交易要帶<T4100> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_Device3(szTemplate);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5581", BAD_CAST szTemplate);

	/* <T5582> : 最後的RRN */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_LastRRN(szTemplate);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5582", BAD_CAST szTemplate);

	/* <T5583> : 最後的DATE */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_LastTransDate(szTemplate);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5583", BAD_CAST szTemplate);

	/* 儲存xml檔 xml檔現在會自動換行了 */
	inXML_SaveFile(_CMAS_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_CMAS_API_REQ_FILE_, _AP_ROOT_PATH_, "", _CMAS_FOLDER_PATH_);

	/* 清空佔用記憶體 */
	inXML_End(&srDoc);

	/* 步驟完成，建立檢核擋 */
	inRetVal = inFILE_Create(&ulHandle, (unsigned char *)_CMAS_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	inFile_Move_File(_CMAS_API_REQ_CHECK_, _FS_DATA_PATH_, "", _CMAS_FOLDER_PATH_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_Void_TOP_UP_ICERAPI() END !");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PACK_Deduct_ICERAPI_FISC
Date&Time       :2020/2/24 下午 1:00
Describe        :組財金XML
Author          :Hachi
*/
int inCMAS_PACK_Deduct_ICERAPI_FISC(TRANSACTION_OBJECT *pobTran)
{
	unsigned char uszMerchantID[15 + 1] = {0};
	unsigned char uszTerminalID[8 + 1] = {0};
	char szTemplate[200 + 1] = {0};
	char szAscii[100 + 1] = {0};
	xmlDocPtr srDoc = NULL;
	xmlNodePtr srRoot_Node = NULL,sr606100_Node = NULL;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_Deduct_ICERAPI_FISC START!!!");
	}

	/*先將HDT對準信用卡 host,因為tid,mid要用到..*/
        /*20210623 Edit start [Hachi]*/
        
        if (inLoadHDTRec(_HDT_INDEX_00_HOST_) != VS_SUCCESS)
		return (VS_ERROR);
        
        /*20210623 Edit start [Hachi]*/
	
        /* 刪除原本的ICERAPI */
	inFile_Unlink_File(_CMAS_API_REQ_FILE2_, _CMAS_FOLDER_PATH_);

	/* 建立一個新的檔案 */
	srDoc = xmlNewDoc(BAD_CAST "1.0");
	srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
	xmlDocSetRootElement(srDoc, srRoot_Node);

	/* 從既有檔案擴增TAG */
	/* <606100> =================================================================================購貨0100 */
	// sr606100_Node = xmlNewNode(NULL, BAD_CAST "606100");
	sr606100_Node = xmlNewNode(NULL, BAD_CAST "000000");
	xmlAddChild(srRoot_Node, sr606100_Node);

	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s%s%s%s%s%s%s%s%s%s", "0000", 			/* 總長度 ,easy card會填,所以保留0000*/
												"6009500000", 		/* TPDU */
												"0100", 			/* MTI */
												"7020010100C000C0",	/* bitmap*/
												"100000000000" ,	/* field 2 ,卡號,前面是長度,固定10,後面固定放0,EASY CARD會自己填入*/
												"811000" ,			/* field 3 , processing code */
												"000000000000",		/* field 4,  amount ,固定放0,EASY CARD會自己填入*/
												"000000",			/* field 11, STAN ,固定放0,EASY CARD會自己填入*/
												"0538" ,			/* field 24, NII */
												"06000001"			/* field 32, IIC,分辨卡種 ,easy card固定000001,06為長度*/
															);
	memset(uszTerminalID, 0x00, sizeof(uszTerminalID));				/*TID*/  			
	inGetTerminalID(((char *)uszTerminalID));
	memset(szAscii, 0x00, sizeof(szAscii));
	inFunc_BCD_to_ASCII(&szAscii[0],&uszTerminalID[0], 8);
	strcat(szTemplate, szAscii);

	if(ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"in %s TID is %s",__FUNCTION__,szAscii);
	}

	memset(uszMerchantID, 0x00, sizeof(uszMerchantID));				/*MID*/
	inGetMerchantID(( (char *)uszMerchantID));
	memset(szAscii, 0x00, sizeof(szAscii));
	inFunc_BCD_to_ASCII(&szAscii[0],&uszMerchantID[0], 15);
	strcat(szTemplate, szAscii);
	xmlNewChild(sr606100_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
	
    if(ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"in %s MID is %s",__FUNCTION__,szAscii);
	}
	
	/* <AftData> */
	xmlNewChild(sr606100_Node, NULL, BAD_CAST "AftData", BAD_CAST "00500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
	
	/* <F02Adr> */
	xmlNewChild(sr606100_Node, NULL, BAD_CAST "F02Adr", BAD_CAST "+18");
	/* <F04Adr> */
	xmlNewChild(sr606100_Node, NULL, BAD_CAST "F04Adr", BAD_CAST "+26");
	/* <F11Adr> */
	xmlNewChild(sr606100_Node, NULL, BAD_CAST "F11Adr", BAD_CAST "+32");
	/* <F58Adr> */
	xmlNewChild(sr606100_Node, NULL, BAD_CAST "F58Adr", BAD_CAST "-2");
	/* <F39Adr> */
	xmlNewChild(sr606100_Node, NULL, BAD_CAST "F39Adr", BAD_CAST "058");
	/* <ECCAdr> */
	xmlNewChild(sr606100_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST "083");


	/* 儲存xml檔 xml檔現在會自動換行了 */
	inXML_SaveFile(_CMAS_API_REQ_FILE2_, &srDoc, "utf-8", 1);
	inFile_Move_File(_CMAS_API_REQ_FILE2_, _AP_ROOT_PATH_, "", _CMAS_FOLDER_PATH_);

	/* 清空佔用記憶體 */
	inXML_End(&srDoc);

	// if (inECC_Insert_Temp() != VS_SUCCESS)
	// {
	// 	return (VS_ERROR);
	// }

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_Deduct_ICERAPI_FISC END!!!");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PACK_Settle_ICERAPI
Date&Time       :2020/2/27 下午 1:00
Describe        :組結帳XML
Author          :Hachi
*/
int inCMAS_PACK_Settle_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	int inESVCIndex = 0;
	char szTemplate[20 + 1] = {0};
	char szSTAN[6 + 1] = {0};
	char szInvoiceNum[6 + 1] = {0};
	char szTicket_NeedNewBatch[2 + 1] = {0};
	char szCMASAddTcpipMode[ 2 + 1 ] ={0};
	unsigned long ulHandle = 0;
	RTC_NEXSYS srRTC = {0};
	xmlDocPtr srDoc = NULL;
	xmlNodePtr srRoot_Node = NULL, srTRANS_Node = NULL, sr5592_Node = NULL;
	TICKET_ACCUM_TOTAL_REC srTicketAccum;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_Settle_ICERAPI START!!!");
	}

	inFunc_Find_Specific_HDTindex( -1, _HOST_NAME_CMAS_, &inESVCIndex);
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inESVCIndex (%d)", inESVCIndex);
	}

	/*20200508 New Load TDT.dat [Hachi]*/
	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
		return (VS_ERROR);

	if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);

	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);
		
	/* 刪除原本的ICERAPI */
	inFile_Unlink_File(_CMAS_API_REQ_CHECK_, _CMAS_FOLDER_PATH_); /* 檢核用 */
	inFile_Unlink_File(_CMAS_API_REQ_FILE_, _CMAS_FOLDER_PATH_);
	inFile_Unlink_File(_CMAS_API_REQ_FILE2_, _CMAS_FOLDER_PATH_);
	inFile_Unlink_File(_CMAS_API_RES_FILE_, _CMAS_FOLDER_PATH_); /* 防止用到上次資料結果 */

	/* Retry時需要參考RES2，不能砍 */
	if (guszECCRetryBit == VS_TRUE)
	{
	}
	else
	{
		inFile_Unlink_File(_CMAS_API_RES_FILE2_, _ECC_FOLDER_PATH_);
	}

	/* 建立一個新的檔案 */
	srDoc = xmlNewDoc(BAD_CAST "1.0");
	srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
	xmlDocSetRootElement(srDoc, srRoot_Node);

	/* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

	/* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0500");

	/* <T0300> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "900099");

	/* <T1100> TM Serial Number */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);

	/* <T1101> : HOST SERIAL NUMBER 20200220 ask Green 說不用送[Hachi] */
	/* 20201216 帶T1101測試 */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetTicket_STAN(szSTAN);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
	inSetTicket_STAN(szTemplate);
	inSaveTDTRec(_TDT_INDEX_01_ECC_);

	memset(szCMASAddTcpipMode, 0x00, sizeof(szCMASAddTcpipMode));
	inGetCMASAddTcpipMode(szCMASAddTcpipMode);

	/* 20200416 AdditionalTcpipData != 0 時 <T1102>，<T1103>需要帶[Hachi]*/	
	if (szCMASAddTcpipMode[0] != '0')
	{
		/* <T1102> 因為財金需要STAN和Invoice num 所以新增*/
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);

		/* <T1103> Invoice num(RRN後三碼) */
		memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
		inGetInvoiceNum(szInvoiceNum);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);
	}
	
	/* <T1200> : hhmmss */
	memset(&srRTC, 0x00, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

	/* <T1300> : yyyymmdd */
	/* hhmmss就抓過一次時間了，不用重抓 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);

	/* <T5501> : Batch Number - 悠遊卡特有，同一天流水號累加，隔天就要歸1 */
	/* 如果szTicket_NeedNewBatch在結帳時為Y */
	memset(szTicket_NeedNewBatch, 0x00, sizeof(szTicket_NeedNewBatch));
	inGetTicket_NeedNewBatch(szTicket_NeedNewBatch);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inCMAS_Batch_Check(szTemplate);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5501", BAD_CAST szTemplate);

	/* <T5591> : 總筆數 */
	memset(&srTicketAccum, 0x00, sizeof(srTicketAccum));
	if (inACCUM_GetRecord_ESVC(pobTran, &srTicketAccum) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintfAt(AT,"GET ECC ACCUM FAIL");
		}
	}
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%ld", srTicketAccum.lnEASYCARD_TotalCount);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5591", BAD_CAST szTemplate);

	/* <T5592> : 總金額 */
	/* <559201>要包在<T5592>內 */
	sr5592_Node = xmlNewNode(NULL, BAD_CAST "T5592");
	xmlAddChild(srTRANS_Node, sr5592_Node);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%lld00", srTicketAccum.llEASYCARD_TotalAmount);
	xmlNewChild(sr5592_Node, NULL, BAD_CAST "T559201", BAD_CAST szTemplate);

	/* 儲存xml檔 xml檔現在會自動換行了 */
	inXML_SaveFile(_CMAS_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_CMAS_API_REQ_FILE_, _AP_ROOT_PATH_, "", _CMAS_FOLDER_PATH_);

	/* 清空佔用記憶體 */
	inXML_End(&srDoc);

	/* 步驟完成，建立檢核擋 */
	inRetVal = inFILE_Create(&ulHandle, (unsigned char *)_CMAS_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	inFile_Move_File(_CMAS_API_REQ_CHECK_, _FS_DATA_PATH_, "", _CMAS_FOLDER_PATH_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_Settle_ICERAPI END!!!");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_PACK_LogOn_ICERAPI
Date&Time       :2020/5/8 下午 5:00
Describe        :組開機LogOn XML
Author          :Hachi
*/
int inCMAS_PACK_LogOn_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	char szHostIndex[2 + 1] = {0};
	char szSTAN[6 + 1] = {0};
	char szInvoiceNum[6 + 1] = {0};
	char szTemplate[20 + 1] = {0};
	unsigned long ulHandle = 0;
	RTC_NEXSYS srRTC = {0};
	xmlDocPtr srDoc = NULL;
	xmlNodePtr srRoot_Node = NULL, srTRANS_Node = NULL;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"----------------------------------------");
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_LogOn_ICERAPI() START !");
	}

	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
		return (VS_ERROR);

	/* MFT index 紀錄 HDT index */
	memset(szHostIndex, 0x00, sizeof(szHostIndex));
	inGetTicket_HostIndex(szHostIndex);
	pobTran->srBRec.inHDTIndex = atoi(szHostIndex);

	if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
		return (VS_ERROR);
	if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
		return (VS_ERROR);


	/* 刪除原本的ICERAPI */
	inFile_Unlink_File(_CMAS_API_REQ_CHECK_, _CMAS_FOLDER_PATH_); /* 檢核用，有表示組ICERAPI成功 */
	inFile_Unlink_File(_CMAS_API_REQ_FILE_, _CMAS_FOLDER_PATH_);  /* 悠遊卡自用，交易資料，每次都要重組 */
	inFile_Unlink_File(_CMAS_API_REQ_FILE2_, _CMAS_FOLDER_PATH_); /* NCCC用，每次重組，但裡面Logon不用 */
	inFile_Unlink_File(_CMAS_API_REQ_TEMP_, _CMAS_FOLDER_PATH_);  /* Logon用，因為CMAS_API_REQ_FILE2交易都要組，所以改寫到暫存檔 */

	inFile_Unlink_File(_CMAS_API_RES_FILE_, _CMAS_FOLDER_PATH_); /* 防止用到上次資料結果 */
	/* Retry時需要參考RES2，不能砍 */
	if (guszECCRetryBit == VS_TRUE)
	{
	}
	else
	{
		inFile_Unlink_File(_CMAS_API_RES_FILE2_, _CMAS_FOLDER_PATH_);
	}

	/* 建立一個新的檔案 */
	srDoc = xmlNewDoc(BAD_CAST "1.0");
	srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
	xmlDocSetRootElement(srDoc, srRoot_Node);

	/* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

	/* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0800");

	/* <T0300> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "881999");

	/* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);

	/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetTicket_STAN(szSTAN);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
	inSetTicket_STAN(szTemplate);
	inSaveTDTRec(_TDT_INDEX_01_ECC_);

	/* <T1102> : NCCC STAN */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);

	/* <T1103> : NCCC RRN(後三碼) */
	memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
	inGetInvoiceNum(szInvoiceNum);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);

	/* <T1200> : hhmmss */
	memset(&srRTC, 0x00, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

	/* <T1300> : yyyymmdd */
	/* hhmmss就抓過一次時間了，不用重抓 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);

	/* 儲存xml檔 xml檔現在會自動換行了 */
	inXML_SaveFile(_ECC_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);

	/* 清空佔用記憶體 */
	inXML_End(&srDoc);

	/* 步驟完成，建立檢核擋 */
	inRetVal = inFILE_Create(&ulHandle, (unsigned char *)_ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _FS_DATA_PATH_, "", _ECC_FOLDER_PATH_);

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintfAt(AT,"inCMAS_PACK_LogOn_ICERAPI() END !");
		inDISP_LogPrintfAt(AT,"----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_ISO_GetSTAN
Date&Time       :2020/6/18 上午 15:28
Describe        :取得STAN號碼
Author          :Hachi
*/
int inCMAS_ISO_GetSTAN(TRANSACTION_OBJECT *pobTran)
{
	char szSTANNum[12 + 1];

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintfAt(AT, "inCMAS_ISO_GetSTAN() START!");

	memset(szSTANNum, 0x00, sizeof(szSTANNum));
	if (inGetSTANNum(szSTANNum) == VS_ERROR)
		return (VS_ERROR);

	pobTran->srBRec.lnSTANNum = atol(szSTANNum);

	return (VS_SUCCESS);
}

/*
Function        :inCMAS_ISO_GetSTAN
Date&Time       :2020/6/18 上午 15:28
Describe        :STAN累加並存回table
Author          :Hachi
*/
int inCMAS_ISO_SetSTAN(TRANSACTION_OBJECT *pobTran)
{
	long lnSTAN;
	char szSTANNum[12 + 1];

	memset(szSTANNum, 0x00, sizeof(szSTANNum));
	if (inGetSTANNum(szSTANNum) == VS_ERROR)
		return (VS_ERROR);

	lnSTAN = atol(szSTANNum);
	if (lnSTAN++ > 999999)
		lnSTAN = 1;

	memset(szSTANNum, 0x00, sizeof(szSTANNum));
	sprintf(szSTANNum, "%06ld", lnSTAN);
	if (inSetSTANNum(szSTANNum) == VS_ERROR)
		return (VS_ERROR);

	if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}
