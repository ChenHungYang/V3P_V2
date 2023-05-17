/* 
 * File:   CMASAPIISO.h
 * Author: Hachi
 *
 * Created on 2020年2月5日, 上午 11:00
 */

#ifndef CMASAPIISO_H
#define CMASAPIISO_H

#ifdef __cplusplus
extern "C"
{
#endif
    
/*
 * AdditionalTcpipData:附加額外的 TCPIP 資料(有額外需附加的資料時,請參照 2.1.6 章節)
 * 0=沒有額外需附加的資料
 * 1=有額外需附加的資料(NCCC)
 * 2=有額外需附加的資料(CTCB)
 * 3=有額外需附加的資料(FISC)
 *
 * 2.1.6 API 參數檔 2(當 AdditionalTcpipData 不等於 0 時,才需要以下檔案)
 * ICERAPI2.REQ(表示以下為 TM 傳給 API 時需準備之檔案)
 * ICERAPI2.RES(表示以下為 API 傳給 TM 時需準備之檔案)
 * 
 *  */	
#define _CMAS_PARA_FILE_                "ICERINI.xml"
#define _CMAS_API_REQ_FILE_             "ICERAPI.REQ"
#define _CMAS_API_REQ_FILE2_            "ICERAPI2.REQ"
#define _CMAS_API_REQ_TEMP_             "ICERTEMP.REQ"
#define _CMAS_API_REQ_TEMP_BAK_         "ICERTEMP.REQ.BAK" /* 處理Temp時使用 */


#define _CMAS_API_RES_FILE_             "ICERAPI.RES"
#define _CMAS_API_RES_FILE2_            "ICERAPI2.RES"
#define _CMAS_API_REQ_CHECK_            "ICERAPI.REQ.OK"
#define _CMAS_API_REQ_RETRY_FILE_       "ICERRETRY.REQ"

#define _CMAS_FOLDER_NAME_              "ICERData"
#define _CMAS_FOLDER_PATH_              "./ICERData/"
#define _CMAS_ICER_LOG_FOLDER_PATH_     "./ICERData/ICERLog/"

#define _CMAS_HEADER_LENTH_     17
#define _CMAS_F03_LENTH_        3
#define _CMAS_F04_LENTH_        6
#define _CMAS_F11_LENTH_        3
#define _CMAS_F12_LENTH_        3
#define _CMAS_F13_LENTH_        2
#define _CMAS_F22_LENTH_        2
#define _CMAS_F24_LENTH_        2
#define _CMAS_F25_LENTH_        1
#define _CMAS_F32_LENTH_        3
#define _CMAS_F35_LENTH_        35
#define _CMAS_F37_LENTH_        12
#define _CMAS_F38_LENTH_        6
#define _CMAS_F39_LENTH_        2
#define _CMAS_F41_LENTH_        8
#define _CMAS_F42_LENTH_        15
#define _CMAS_F48_LENTH_        21
#define _CMAS_F56_LENTH_        6
//#define _CMAS_F59_ET_LENTH_	34      /* 這裡指Table "ET" Data位置，前面有Length(2)+N1(6)+N2(6)+N3(8)+NF(8)+ET(4) = 34 */
#define _CMAS_F62_LENTH_        72 /* F_59Length(2)+F_59(60)+F_60Length(2)+F_60(6)+F_62Length(2) = 72 */

#define _XML_TAG_TYPE_NONE_     0
#define _XML_TAG_TYPE_START_    1
#define _XML_TAG_TYPE_END_      2

#define _CMAS_DEFAULT_BATCH_    "00000001"

/*PacketLenFlag*/
#define _CMAS_PACKET_LEN_ON_    0
#define _CMAS_PACKET_LEN_OFF_   1

/*AdditionalTcpipData */
#define _CMAS_NONE_TCPIP_DATA_  0
#define _CMAS_NCCC_TCPIP_DATA_  1
#define _CMAS_CTCB_TCPIP_DATA_  2
#define _CMAS_FISC_TCPIP_DATA_  3

	int inCMAS_PACK_ICERINI(TRANSACTION_OBJECT *pobTran);
	int inCMAS_PACK_Deduct_ICERAPI(TRANSACTION_OBJECT *pobTran);
	int inCMAS_UNPACK_ICERAPI(TRANSACTION_OBJECT *pobTran, char *szFileName);
	int inCMAS_PACK_Refund_ICERAPI(TRANSACTION_OBJECT *pobTran);
	int inCMAS_PACK_Inquiry_ICERAPI(TRANSACTION_OBJECT *pobTran);
	int inCMAS_PACK_TOP_UP_ICERAPI(TRANSACTION_OBJECT *pobTran);
	int inCMAS_PACK_Void_TOP_UP_ICERAPI(TRANSACTION_OBJECT *pobTran);
	int inCMAS_PACK_Deduct_ICERAPI_FISC(TRANSACTION_OBJECT *pobTran);
	int inCMAS_PACK_Settle_ICERAPI(TRANSACTION_OBJECT *pobTran);
	int inCMAS_PACK_LogOn_ICERAPI(TRANSACTION_OBJECT *pobTran);
	int inCMAS_ISO_GetSTAN(TRANSACTION_OBJECT *pobTran);
	int inCMAS_ISO_SetSTAN(TRANSACTION_OBJECT *pobTran);

#ifdef __cplusplus
}
#endif

#endif /* CMASAPIISO_H */
