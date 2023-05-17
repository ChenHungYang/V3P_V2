
#ifndef __ECC_H__
#define __ECC_H__


#ifdef __cplusplus
extern "C"
{
#endif

#define _ECC_PARA_FILE_		"ICERINI.xml"
#define _ECC_API_REQ_FILE_	"ICERAPI.REQ"
#define _ECC_API_REQ_FILE2_	"ICERAPI2.REQ"  
#define _ECC_API_REQ_TEMP_	"ICERTEMP.REQ"
#define _ECC_API_REQ_TEMP_BAK_	"ICERTEMP.REQ.BAK"	/* 處理Temp時使用 */

#define _ECC_API_RES_FILE_		"ICERAPI.RES"
#define _ECC_API_RES_FILE2_		"ICERAPI2.RES"
#define _ECC_API_REQ_CHECK_		"ICERAPI.REQ.OK"
#define _ECC_API_REQ_RETRY_FILE_	"ICERRETRY.REQ"

#define _ECC_FOLDER_NAME_		"ICERData"
#define _ECC_FOLDER_PATH_		"./ICERData/"
#define _ECC_ICER_LOG_FOLDER_PATH_	"./ICERData/ICERLog/"

#define _ECC_HEADER_LENTH_       17
#define _ECC_F03_LENTH_		3
#define _ECC_F04_LENTH_		6
#define _ECC_F11_LENTH_		3
#define _ECC_F12_LENTH_		3
#define _ECC_F13_LENTH_		2
#define _ECC_F22_LENTH_		2
#define _ECC_F24_LENTH_		2
#define _ECC_F25_LENTH_		1
#define _ECC_F32_LENTH_		3
#define _ECC_F35_LENTH_		35
#define _ECC_F37_LENTH_		12
#define _ECC_F38_LENTH_		6
#define _ECC_F39_LENTH_		2
#define _ECC_F41_LENTH_		8
#define _ECC_F42_LENTH_		15
#define _ECC_F48_LENTH_		21
#define _ECC_F56_LENTH_		6
//#define _ECC_F59_ET_LENTH_	34      /* 這裡指Table "ET" Data位置，前面有Length(2)+N1(6)+N2(6)+N3(8)+NF(8)+ET(4) = 34 */ 
#define _ECC_F62_LENTH_		72      /* F_59Length(2)+F_59(60)+F_60Length(2)+F_60(6)+F_62Length(2) = 72 */

#define _XML_TAG_TYPE_NONE_	0
#define _XML_TAG_TYPE_START_	1
#define _XML_TAG_TYPE_END_	2

#define _ECC_DEFAULT_BATCH_	"00000001"

typedef struct
{
	char	szT1100[6 + 1];
	char	szT1101[6 + 1];
	char	szT1102[6 + 1];
	char	szT1103[6 + 1];
}ECC_RETRY_DATA;


#ifdef __cplusplus
}
#endif

#endif

