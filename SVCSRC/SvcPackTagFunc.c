
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"

#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/HDPT.h"

#include "SvcTable.h"
#include "SvcPackTagFunc.h"


/*
 * Function		: inSVC_PackTagDF66
 * Date&Time	: 2022/9/19 下午 2:20
 * Describe		:
 * Tag			: 0x FF 21 (LMS Processing Code)
 * Len			: 0x 02
 * Description	: LMS處理代碼
 */
int inSVC_PackTagDF66(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;
	char	szVersionAsc[6 + 1];
	char	szVersionBcd[3 + 1];
	char	szVersionDateTimeAsc[14 + 1];
	char	szVersionDateTimeBcd[7 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xDF\x66", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x0A;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	/* Parameter Version */
	memset(szVersionAsc, 0x00, sizeof(szVersionAsc));
	memset(szVersionBcd, 0x00, sizeof(szVersionBcd));
	inGetSVCParameterVersion(szVersionAsc);
	inDISP_LogPrintfWithFlag(" SVC Pack Iso ParaVersion[%s] ", szVersionAsc);
	inFunc_ASCII_to_BCD((unsigned char*)szVersionBcd, szVersionAsc,  3);
	memcpy(&uszPackBuf[inPackTemp], szVersionBcd, 3);
	inPackTemp = inPackTemp + 3;

	/* Parameter Date Time */
	memset(szVersionDateTimeAsc, 0x00, sizeof(szVersionDateTimeAsc));
	memset(szVersionDateTimeBcd, 0x00, sizeof(szVersionDateTimeBcd));
	inGetSVCParameterDateTime(szVersionDateTimeAsc);
	inDISP_LogPrintfWithFlag(" SVC Pack Iso ParaTime[%s] ", szVersionDateTimeAsc);
	inFunc_ASCII_to_BCD((unsigned char*)szVersionDateTimeBcd, szVersionDateTimeAsc,  7);
	memcpy(&uszPackBuf[inPackTemp], szVersionDateTimeBcd, 7);
	inPackTemp = inPackTemp + 7;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
	
}

/*
 * Function		: inSVC_PackTagFF21
 * Date&Time	: 2022/9/19 下午 2:20
 * Describe		:
 * Tag			: 0x FF 21 (LMS Processing Code)
 * Len			: 0x 02
 * Description	: LMS處理代碼
 */
int inSVC_PackTagFF21(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x21", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x02;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		switch (pobTran->srBRec.inCode)
		{
			case _SVC_REDEEM_:
			case _SVC_BACKCARD_:
				memcpy(&uszPackBuf[inPackTemp], "\x76\x48", 2);
				break;
			case _SVC_RELOAD_:
			case _SVC_REFUND_:
				memcpy(&uszPackBuf[inPackTemp], "\x77\x08", 2);
				break;
			default:
				break;
		}
	}
	else
	{
		switch (pobTran->srBRec.inCode)
		{
			case _SVC_TMS_:/* 參數下載 */
				memcpy(&uszPackBuf[inPackTemp], "\x73\x37", 2);
				break;
			case _SVC_TMS_FINISH_:/* 參數下載 */
				memcpy(&uszPackBuf[inPackTemp], "\x73\x37", 2);
				break;
			case _SVC_REDEEM_:/* 電子錢包消費 */
				memcpy(&uszPackBuf[inPackTemp], "\x76\x47", 2);
				break;
			case _SVC_BACKCARD_:/* 退卡 */
				memcpy(&uszPackBuf[inPackTemp], "\x75\x77", 2);
				break;
			case _SVC_RELOAD_:/* 現金加值 */
			case _SVC_REFUND_:/* 退貨 */
				memcpy(&uszPackBuf[inPackTemp], "\x77\x07", 2);
				break;
			case _SVC_ACTIVECARD_:/* Mifare UL 卡片初始化交易 */
				memcpy(&uszPackBuf[inPackTemp], "\x71\x47", 2);
				break;
			case _SVC_SETTLE_: /* 結帳 */
				memcpy(&uszPackBuf[inPackTemp], "\x73\x27", 2);
				break;
			case _SVC_CLS_BATCH_: /* 結帳 */
				memcpy(&uszPackBuf[inPackTemp], "\x73\x27", 2);
				break;
			case _SVC_INQUIRY_: /* 餘額查詢 */
				memcpy(&uszPackBuf[inPackTemp], "\x75\x67", 2);
				break;
			default:
				break;
		}
	}

	inPackTemp = inPackTemp + 2;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF22
 * Date&Time	: 2022/9/19 下午 2:20
 * Describe		:
 * Tag			: 0x FF 22 (Batch Number)
 * Len			: 0x 03
 * Description	: 批號
 */
int inSVC_PackTagFF22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;
	char	szBatchAsc[6 + 1];
	char	szBatchBcd[3 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x22", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x03;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	memset(szBatchAsc,0x00,sizeof(szBatchAsc));
	if (pobTran->srBRec.inCode == _SVC_TMS_)
	{
		sprintf(szBatchAsc, "%06ld", 0L);
	}else{
		inGetBatchNum(szBatchAsc);
	}

	memset(szBatchBcd,0x00,sizeof(szBatchBcd));
	inFunc_ASCII_to_BCD((unsigned char*)szBatchBcd, szBatchAsc,  3);

	memcpy(&uszPackBuf[inPackTemp], szBatchBcd, 3);
	inPackTemp = inPackTemp + 3;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF23
 * Date&Time	: 2022/9/19 下午 2:20
 * Describe		:
 * Tag			: 0x FF 23 (Issuer Number)
 * Len			: 0x 04
 * Description	: 發卡單位代號
 */
int inSVC_PackTagFF23(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x23", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x04;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szSvcIssueId, 4);
	inPackTemp = inPackTemp + 4;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF24
 * Date&Time	: 2022/9/19 下午 2:20
 * Describe		:
 * Tag			: 0x FF 24 (Store Counter Area)
 * Len			: 0x 12
 * Description	: 專櫃相關資訊
 * Store Counter ID	an	5	5	專櫃代號
 * 機號			an	6	6	POS機號
 * 序號			an	6	6	POS交易序號
 * Space			an	1	1	空白

 */
int inSVC_PackTagFF24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x24", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x12;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szStoreID, 18);
	inPackTemp = inPackTemp + 18;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF25
 * Date&Time	: 2022/9/19 下午 2:20
 * Describe		:
 * Tag			: 0x FF 25 (LMS Card Number)
 * Len			: VAR, 0x 00..0x 0B
 * Description	: LMS卡號
 */
int inSVC_PackTagFF25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0, inCardLen = 0 ,inCardOrgLen ,i = 0, inCardBcdLen = 0;
	char	szCardAscData[24 + 1];
	char	szCardNumber[10 + 1];
	char	szMaskAsc[20 + 1];
	char	szMask[10 + 1];
	char	szMaskResult[10 + 1];
	char	szDigitalBuffer[2 + 1];
	char	szDigitalBcdBuffer[1 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x25", 2);
	inPackTemp = inPackTemp + 2;

	memset(szCardAscData,0x00,sizeof(szCardAscData));
	inFunc_BCD_to_ASCII(szCardAscData,(unsigned char*)pobTran->srBRec.szSvcCardNumber, 6);

	inCardOrgLen = strlen (szCardAscData);

	/* 去掉F */
	for ( i = 0 ;i < inCardOrgLen; i++)
	{
		if ( szCardAscData[i] == 'F')
			szCardAscData[i] = 0x00;
	}

        inCardLen = strlen(szCardAscData) ;

        inDISP_LogPrintfWithFlag("inCB_SVC_PackTagFF25()inCardLen [%ld]", inCardLen);

        /* Tag Length */
        uszPackBuf[inPackTemp] = ((inCardLen / 2) + 1) % 65535;
        inDISP_LogPrintfWithFlag("inCB_SVC_PackTagFF25()Tag Len [%d]", uszPackBuf[inPackTemp]);
        inPackTemp = inPackTemp + 1;

	/* Tag Value */
        /* LMS Card Number Digits */
        memset(szDigitalBuffer,0x00,sizeof(szDigitalBuffer));
        memset(szDigitalBcdBuffer,0x00,sizeof(szDigitalBcdBuffer));
        sprintf(szDigitalBuffer,"%02d",inCardLen);
        inFunc_ASCII_to_BCD((unsigned char*)szDigitalBcdBuffer, szDigitalBuffer, 1);
        uszPackBuf[inPackTemp] = szDigitalBcdBuffer[0];
        inPackTemp = inPackTemp + 1;

	/* LMS Card Number */
        memset(szCardNumber,0x00,sizeof(szCardNumber));
	if ( inCardLen < 21 )
	{
		if ( (inCardLen % 2) == 0)
		{
			inCardBcdLen = inCardLen / 2;
			inFunc_ASCII_to_BCD((unsigned char*)szCardNumber, szCardAscData, inCardBcdLen);
		}
		else
		{
			inCardBcdLen = (inCardLen / 2) + 1;
			inFunc_ASCII_to_BCD((unsigned char*)szCardNumber, szCardAscData, inCardBcdLen);
		}
	}
	else
	{
		inCardBcdLen = 10;
		inFunc_ASCII_to_BCD((unsigned char*)szCardNumber, szCardAscData, inCardBcdLen);
	}

	memset(szMaskAsc, 0x00, sizeof(szMaskAsc));
	memset(szMask, 0x00, sizeof(szMask));
	
	inGetSVCMaskData(szMaskAsc);
	inFunc_ASCII_to_BCD((unsigned char*)szMask, szMaskAsc, 10);

	for ( i = 0 ;i < inCardBcdLen ; i++)
	{
		szMaskResult[i] = szCardNumber[i] ^ szMask[i + (10 - inCardBcdLen)];
	}

	memcpy(&uszPackBuf[inPackTemp], szMaskResult, inCardBcdLen);
	inPackTemp = inPackTemp + inCardBcdLen;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF26
 * Date&Time	: 2022/9/19 下午 2:20
 * Describe		:
 * Tag			: 0x FF 26 (LMS Card Expiration Date)
 * Len			: 0x 04
 * Description	: LMS卡號有效期
 */
int inSVC_PackTagFF26(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x26", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x04;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szSvcExpireDay, 4);
	inPackTemp = inPackTemp + 4;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF29
 * Date&Time	: 2022/9/19 下午 2:20
 * Describe		:
 * Tag			: 0x FF 29 (Terminal Transaction Date Time)
 * Len			: 0x 07
 * Description	: 端末交易日期時間
 */
int inSVC_PackTagFF29(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;
	char	szDateTimeBcd[7 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x29", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x07;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	memset(szDateTimeBcd,0x00,sizeof(szDateTimeBcd));
	inFunc_ASCII_to_BCD((unsigned char*)szDateTimeBcd, pobTran->srBRec.szSvcTxnDateTime, 7);
	memcpy(&uszPackBuf[inPackTemp], szDateTimeBcd, 7);
	inPackTemp = inPackTemp + 7;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF2B
 * Date&Time	: 2022/9/19 下午 2:20
 * Describe		:
 * Tag			: 0x FF 2B (LMS Transaction Amount)
 * Len			: 0x 05
 * Description	: LMS交易金額
 */
int inSVC_PackTagFF2B(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;
	int	i = 0;
	int	inZeofCnt = 0;//2014 0213 記錄補0的Byte長度.
	char    szAmtBuf[ 5 * 2 + 1], szAmtBcdBuf[ 5 + 1 ];
	char	szMaskAsc[20 + 1];
	char	szMask[10 + 1];
	char	szMaskResult[10 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x2B", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x05;
	inPackTemp = inPackTemp + 1;

	memset(szAmtBuf,0x00,sizeof(szAmtBuf));
	sprintf(szAmtBuf, "%ld", pobTran->srBRec.lnTxnAmount*100);
	inZeofCnt = ( 10 - ( strlen( szAmtBuf ) % 11 ) );// 計算補0的個數.
	inZeofCnt /= 2;// 計算補0的Byte長度.
//        pad (szAmtBuf, szAmtBuf, '0', 10, RIGHT);
	inFunc_ASCII_to_BCD((unsigned char*)szAmtBcdBuf, szAmtBuf, 5);
	
	memset(szMaskAsc, 0x00, sizeof(szMaskAsc));
	inGetSVCMaskData(szMaskAsc);
	inDISP_LogPrintfWithFlag("szMaskAsc(%s)",szMaskAsc);

	memset(szMask,0x00,sizeof(szMask));
	inFunc_ASCII_to_BCD((unsigned char*)szMask, szMaskAsc, 10);

	inDISP_LogPrintfWithFlag("szMask(%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X)",szMask[0],szMask[1],szMask[2],szMask[3],szMask[4],szMask[5],szMask[6],szMask[7],szMask[8],szMask[9]);
	inDISP_LogPrintfWithFlag("szAmtBcdBuf(%02X%02X%02X%02X%02X)",szAmtBcdBuf[0],szAmtBcdBuf[1],szAmtBcdBuf[2],szAmtBcdBuf[3],szAmtBcdBuf[4]);

	memset(szMaskResult,0x00,sizeof(szMaskResult));

	for ( i = 0 ;i < 5 ; i++)
	{
		szMaskResult[i] = szAmtBcdBuf[i] ^ szMask[i+5];
	}

	if( 0 < inZeofCnt )//2014 0213
	{
		memset( szMaskResult, 0x00, inZeofCnt );//2014 0213 補0的部份不用加密, 補回0.
	}

	memcpy((char *)&uszPackBuf[inPackTemp], &szMaskResult[0], 5 );
	inPackTemp = inPackTemp + 5;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}


/*
 * Function		: inSVC_PackTagFF2E
 * Date&Time	: 2022/9/20 上午 10:34
 * Describe		:
 * Tag			: 0x FF 2E (Currency Code)
 * Length		: 0x 02
 * Description	: 
 * This value is the 3-digit numeric code used to determine the number of decimal places in fields 4 and fields 58.
 *  A leading zero is required to pad the first unused half-byte of this field. The zero is a filler,
 *  and is not part of the currency code. 
 */
int inSVC_PackTagFF2E(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x2E", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x02;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	memcpy(&uszPackBuf[inPackTemp],"\x09\x01",2);
	inPackTemp = inPackTemp + 2;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}
	
/*
 * Function		: inSVC_PackTagFF2F
 * Date&Time	: 2022/9/20 上午 10:34
 * Describe		:
 * Tag			: 0x FF 2F (LMS Invoice Number)
 * Len			: 0x 06
 * Description	: LMS Invoice Number = Field_58.0xFF29(MMDDHHMMSS) + Field_11(後二碼)
 */
int inSVC_PackTagFF2F(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;
	char	DataAscBuffer[12 + 1];
	char	szSTANAsc[6 + 1];
	char	DataBcdBuffer[6 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x2F", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x06;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	memset(DataAscBuffer, 0x00, sizeof(DataAscBuffer));
	memcpy(&DataAscBuffer[0] ,&pobTran->srBRec.szSvcTxnDateTime[4],10);//MMDDHHMMSS
	memset(szSTANAsc, 0x00, sizeof(szSTANAsc));
	sprintf(szSTANAsc, "%06ld", pobTran->srBRec.lnSTANNum);
	memcpy(&DataAscBuffer[10],&szSTANAsc[4],2);//Stan number end 2 digit

	memset(DataBcdBuffer, 0x00, sizeof(DataBcdBuffer));
	inFunc_ASCII_to_BCD((unsigned char*)&DataBcdBuffer[0], &DataAscBuffer[0], 6);

	memcpy(&uszPackBuf[inPackTemp], &DataBcdBuffer[0], 6);
	inPackTemp = inPackTemp + 6;


	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF31
 * Date&Time	: 2022/9/20 上午 10:34
 * Describe		:
 * Tag		: 0x FF 31 (Reload/Redeem Program ID)
 * Len		: 0x 02
 * Description	:
 * 加值交易, 此Tag代表加值方案代碼:
 * 	Reload Program ID=‘0000’ : 根據Tag [0x FF 2B]帶上來的交易金額進行加值, 並計算紅利.
 * 	Reload Program ID=Other : 根據此加值方案代號, 由主機找出相對應的加值金額進行加值.
 * 抵扣交易時, 此Tag代表抵扣方案代碼:
 * 	Redeem Program ID=‘0000’ : 根據Tag [0x FF 36] OR Tag [0x FF 3C]帶上來的交易金額進行抵扣.
 * 	Redeem Program ID=Other : 根據此抵扣方案代號, 由主機找出相對應的抵扣金額進行抵扣.
 */
int inSVC_PackTagFF31(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x31", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x02;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	memcpy(&uszPackBuf[inPackTemp], "\x00\x00", 2 );
	inPackTemp = inPackTemp + 2;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF3C
 * Date&Time	: 2022/9/20 上午 10:34
 * Describe		:
 * Tag			: 0x FF 3C (Host Points Transaction Area)
 * Len			: 0x 00..0x 14
 * Description	: 主機點數抵扣資訊
 */
int inSVC_PackTagFF3C(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;
	char	szAmtBuf[ 5 * 2 + 1];
	char	DataAscBuffer[40 + 1];
	char	DataBcdBuffer[20 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x3C", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x14;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	memset(szAmtBuf,0x00,sizeof(szAmtBuf));
	sprintf(szAmtBuf, "%010ld", pobTran->srBRec.lnTxnAmount*100);
//        pad (szAmtBuf, szAmtBuf, '0', 10, RIGHT);

	memset(DataAscBuffer, 0x00, sizeof(DataAscBuffer));
	memset(&DataAscBuffer[0], 0x30, 40);
	memcpy(&DataAscBuffer[0], szAmtBuf, 10);

	memset(DataBcdBuffer, 0x00, sizeof(DataBcdBuffer));
	inFunc_ASCII_to_BCD((unsigned char*)DataBcdBuffer, DataAscBuffer, 20);
	memcpy(&uszPackBuf[inPackTemp], DataBcdBuffer, 20);
	inPackTemp = inPackTemp + 20;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF45
 * Date&Time	: 2022/9/20 上午 10:34
 * Describe		:
 * Tag			: 0x FF 45 (Access Mode)
 * Len			: 0x 01
 * Description	: 交易之讀卡方式: 接觸式Contect: C 非接觸式ContestLess: L
 */
int inSVC_PackTagFF45(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	
	int	inPackTemp = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x45", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Data Length */
	uszPackBuf[inPackTemp] = 0x01;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	uszPackBuf[inPackTemp] = 0x4C; /* 非接觸式ContestLess: L */
	inPackTemp = inPackTemp + 1;
        
	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF4B
 * Date&Time	: 2022/9/20 上午 10:34
 * Describe		:
 * Tag			: 0x FF 4B (Host Bonus Inquiry)
 * Len			: 0x 06
 * Description	: 此欄位用於主機紅利餘額查詢交易, 告知LMS主機要查詢點數或點券餘額
 */
int inSVC_PackTagFF4B(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x4B", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x06;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	/* Bonus Type + Coupon ID */
	/* 查詢紅利型態(點數/點券) P: 點數(Points) C: 點券(Coupon) A: 點數+點券 X: 點數(含未生效)+ 點券(含未生效) */
	/* If Type=A ID=”0000000000” */
	memcpy(&uszPackBuf[inPackTemp], "\x41\x00\x00\x00\x00\x00", 6 );
	inPackTemp = inPackTemp + 6;


	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
	
}


/*
 * Function		: inSVC_PackTagFF61
 * Date&Time	: 2022/9/20 上午 10:34
 * Describe		:
 * Tag			: 0x FF 61 (Batch Date)
 * Len			: 0x 04
 * Description	: 批次歸屬日期
 */
int inSVC_PackTagFF61(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;
	char	szDateTimeBcd[4 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x61", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x04;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	memset(szDateTimeBcd,0x00,sizeof(szDateTimeBcd));
	inFunc_ASCII_to_BCD((unsigned char*)szDateTimeBcd, pobTran->srBRec.szSvcTxnDateTime, 7);

	memcpy(&uszPackBuf[inPackTemp], szDateTimeBcd, 4);
	inPackTemp = inPackTemp + 4;
		
	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
	
}


/*
 * Function		: inSVC_PackTagFF63
 * Date&Time	: 2022/9/20 上午 10:34
 * Describe		:
 * Tag		: 0x FF 63 (Settlement Batch Totals Information)
 * Len		: 0x 2A
 */
int inSVC_PackTagFF63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
// TODO: 決定要怎麼存之後再補欄位，因為要讀取金額，所以要整個重弄
//	int	inPackTemp = 0 ,inBatchCnt = 0 , inAccumCnt = 0;
//	char	szTagTempBuffer[100 + 1];
//	char	szBuffer[84 + 1];
//	char	szSvcCountAmtAsc[16+1];
//	SVC_ACCUM_TOTAL_REC srAccumRec;
//
//	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
//
//	memset(szTagTempBuffer,0x00,sizeof(szTagTempBuffer));
//
//	/* Tag Name */
//	memcpy(&szTagTempBuffer[inCnt], "\xFF\x63", 2);
//	inCnt = inCnt + 2;
//
//	/* Tag Length */
//	szTagTempBuffer[inCnt] = 0x2A;
//	inCnt = inCnt + 1;
//
//	/* Tag Value */
//	memset(szBuffer, 0x30, 84);
//	inBatchCnt = inBATCH_CheckBatchKeyFile(pobTran);
//	if (inBatchCnt == VS_ERROR)
//		return (VS_ERROR);
//	else if (inBatchCnt == VS_NO_RECORD)
//	{
//		SVC_DSP_2_HEX(szBuffer, &szTagTempBuffer[inCnt], 42);
//		inCnt = inCnt + 42;
//	}
//	else
//	{
//		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
//		inAccumCnt = inACCUM_GetRecord_SVC(pobTran, &srAccumRec);
//		if (inAccumCnt == VS_NO_RECORD)
//		{
//			SVC_DSP_2_HEX(szBuffer, &szTagTempBuffer[inCnt], 42);
//			inCnt = inCnt + 42;
//		}
//		else if (inAccumCnt == VS_ERROR)
//			return (VS_ERROR);
//		else
//		{
//			memset(szSvcCountAmtAsc,0x00,sizeof(szSvcCountAmtAsc));
//			sprintf(szSvcCountAmtAsc,"%06ld%010ld",srAccumRec.lnTotalCount, (long)srAccumRec.dbTotalAmount*100);
//			memcpy(&szBuffer[52],szSvcCountAmtAsc,16);
//			SVC_DSP_2_HEX(szBuffer, &szTagTempBuffer[inCnt], 42);
//			inCnt = inCnt + 42;
//		}
//	}

//	 *inPackAddr += inPackTemp;
//	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
//	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF64
 * Date&Time	: 2022/9/20 上午 10:34
 * Describe		:
 * Tag			: 0x FF 64 (Upload/Download Controller)
 * Len			: 0x 08
 * Description	: 檔案上傳/下載控制資訊
 */
int inSVC_PackTagFF64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;
	char	szCntAsc[4 + 1];
	char	szCntBcd[2 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x64", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x08;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	memset(szCntAsc,0x00,sizeof(szCntAsc));
	sprintf(szCntAsc, "%04d", pobTran->inSvcTotalCnt);
	memset(szCntBcd,0x00,sizeof(szCntBcd));
	inFunc_ASCII_to_BCD((unsigned char*)szCntBcd, szCntAsc, 2);
	memcpy(&uszPackBuf[inPackTemp], szCntBcd, 2);
	inPackTemp = inPackTemp + 2;

	memset(szCntAsc, 0x00, sizeof(szCntAsc));
	sprintf(szCntAsc, "%04d", pobTran->inSvcSendCnt);
	memset(szCntBcd, 0x00, sizeof(szCntBcd));
	inFunc_ASCII_to_BCD((unsigned char*)szCntBcd, szCntAsc, 2);
	memcpy(&uszPackBuf[inPackTemp], szCntBcd, 2);
	inPackTemp = inPackTemp + 2;

	memset(szCntAsc, 0x00, sizeof(szCntAsc));
	sprintf(szCntAsc, "%04d", pobTran->inSvcReceiveCnt);
	memset(szCntBcd, 0x00, sizeof(szCntBcd));
	inFunc_ASCII_to_BCD((unsigned char*)szCntBcd, szCntAsc, 2);
	memcpy(&uszPackBuf[inPackTemp], szCntBcd, 2);
	inPackTemp = inPackTemp + 2;

	memset(szCntAsc, 0x00, sizeof(szCntAsc));
	sprintf(szCntAsc, "%04d", pobTran->inSvcRecordCnt);
	memset(szCntBcd, 0x00, sizeof(szCntBcd));
	inFunc_ASCII_to_BCD((unsigned char*)szCntBcd, szCntAsc, 2);
	memcpy(&uszPackBuf[inPackTemp], szCntBcd, 2);
	inPackTemp = inPackTemp + 2;
	
	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

/*
 * Function		: inSVC_PackTagFF65
 * Date&Time	: 2022/9/20 上午 10:34
 * Describe		:
 * Tag			: 0x FF 65 (Mifare UL UID)
 * Len			: 0x 07
 * Description	: Mifare UL UID
 */
int inSVC_PackTagFF65(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x65", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Length */
	uszPackBuf[inPackTemp] = 0x07;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	memcpy(&uszPackBuf[inPackTemp], pobTran->srBRec.szSvcUID, 7);
	inPackTemp = inPackTemp + 7;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}



/*
 * Function		: inSVC_PackTagFF6E
 * Date&Time	: 2022/9/20 上午 10:34
 * Describe		:
 * Tag			: 0x FF 6E (Transaction Type)
 * Len			: 0x 01
 * Description	: 此欄位用於批次上傳交易, 告知LMS主機此交易是online 或 offline.
 */
int inSVC_PackTagFF6E(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr)
{
	int	inPackTemp = 0;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	/* Tag Name */
	memcpy(&uszPackBuf[inPackTemp], "\xFF\x6E", 2);
	inPackTemp = inPackTemp + 2;

	/* Tag Data Length */
	uszPackBuf[inPackTemp] = 0x01;
	inPackTemp = inPackTemp + 1;

	/* Tag Value */
	/* Transaction Type代表意義如下:
	N: online transaction(0x4E)
	F: offline transaction(0x46)
	*/
	uszPackBuf[inPackTemp] = 0x4E;
	inPackTemp = inPackTemp + 1;

	*inPackAddr += inPackTemp;
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return VS_SUCCESS;
}

