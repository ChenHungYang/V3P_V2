#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../FUNCTION/Batch.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/KMS.h"
#include "../EVENT/MenuMsg.h"

#include "ProcessTmk.h"
#include "deslib.h"

extern  int     ginDebug;       /* Debug使用 extern */
extern  int	ginISODebug;
extern unsigned char guszMKCheckValue[24];
extern unsigned char guszWKkCheckValue[24];

KEY_TABLE TMK_KEY_TABLE[] =
{
	{_MULTI_TMK_DATA_1_},
	{_MULTI_TMK_DATA_2_},
	{_MULTI_TMK_DATA_3_},
	{_MULTI_TMK_DATA_4_},
	{_MULTI_TMK_DATA_5_},
	{_MULTI_TMK_DATA_6_},
	{_MULTI_TMK_DATA_7_},
	{_MULTI_TMK_DATA_8_},
	{_MULTI_TMK_DATA_9_},
	{_MULTI_TMK_DATA_10_},
	{_MULTI_TMK_DATA_11_},
	{_MULTI_TMK_DATA_12_},
	{_MULTI_TMK_DATA_13_},
	{_MULTI_TMK_DATA_14_},
	{_MULTI_TMK_DATA_15_},
};

KCV_TABLE TMK_KCV_TABLE[] =
{
	{_MULTI_TMK_KCV_1_},
	{_MULTI_TMK_KCV_2_},
	{_MULTI_TMK_KCV_3_},
	{_MULTI_TMK_KCV_4_},
	{_MULTI_TMK_KCV_5_},
	{_MULTI_TMK_KCV_6_},
	{_MULTI_TMK_KCV_7_},
	{_MULTI_TMK_KCV_8_},
	{_MULTI_TMK_KCV_9_},
	{_MULTI_TMK_KCV_10_},
	{_MULTI_TMK_KCV_11_},
	{_MULTI_TMK_KCV_12_},
	{_MULTI_TMK_KCV_13_},
	{_MULTI_TMK_KCV_14_},
	{_MULTI_TMK_KCV_15_},
};

/*	Common KeySets（shared key 不因砍程式後消失）
 * 	KMS2_COMMON_KEY_SETS_START				0xC000
 * 	KMS2_COMMON_KEY_SETS_END				0xCFFF
 *
 *	不能使用以下位置
 * 	Reserved Keys for System
 * 	KMS2_RESERVED_KEY_SETS_0000				0x0000
 * 	KMS2_RESERVED_KEY_SETS_START				0xFF00
 * 	KMS2_RESERVED_KEY_SETS_END				0xFFFF
 *
 *	KMS筆記: keyset:0xC000 ~ 0xCFFF 是shared keyset 不會因為程式刪除而消失，且不能使用delete key fuction刪除
 *	    (delete all key只砍該owner的所有key，但0xC001 ~ 0xCFFF沒有owner (shared key是共用的()
 */

/*
Function        :inNCCC_TMK_Write_Test_TerminalMasterKey_Flow
Date&Time       :2017/1/13 下午 4:08
Describe        :
*/
int inNCCC_TMK_Write_Test_TerminalMasterKey_Flow()
{
	char	szKey;

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont_Color("1.直接覆寫銀聯測試Key", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("2.和暫存Key交換", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("3.查詢Key", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("4.查看Master Key 狀態", _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_BLACK_, _DISP_LEFT_);

		szKey = uszKBD_GetKey(30);
		if (szKey == _KEY_1_)
		{
			inNCCC_TMK_Write_Test_TerminalMasterKey();
			break;
		}
		else if (szKey == _KEY_2_)
		{
			inNCCC_TMK_ProductionKey_Swap_To_Temp();
			break;
		}
		else if (szKey == _KEY_3_)
		{
			inKMS_GetKeyInfo_LookUp();
		}
		else if (szKey == _KEY_4_)
		{
			inNCCC_TMK_GetKeyInfo_LookUp_Default();
		}
		else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
		{
			break;
		}

	}

	return (VS_SUCCESS);
}

/*
 Function	:inNCCC_TMK_Write_Test_TerminalMasterKey
 Date&Time	:2016/2/5 下午 2:39
 Describe	:寫測試MasterKey的function
 *para.Version:			Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:		key set(一個key set只能有一種key type，EX:只能3DES or RSA)
 *para.Info.KeyIndex:		key index
 *para.Info.KeyType:		key之後會用在哪一種加密方法(EX:3DES or RSA......等)
 *para.Info.KeyVersion:		使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:	若有多個屬性，用"or"把bit on起來（KMS2_KEYATTRIBUTE_KPK表示是用來加密其他key的key）
 *para..Protection.Mode:	key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_PLAINTEXT 表示明碼寫入）
*/
int inNCCC_TMK_Write_Test_TerminalMasterKey()
{
	int			i;
	char			szDebugMsg[100 + 1];
	char			szDispMsg[30 + 1];
	char			szKeyData[48 + 1];
	unsigned char		uszHex[24 + 1];		/* 3DES的Key長度最長24byte，若只輸入16byte則k1,k2,k3中，k3 = k1 */
	unsigned short		usKeyLength;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_Test_TerminalMasterKey START()！");
		inDISP_LogPrintf(szDebugMsg);
	}

	memset(&srKeyWritePara, 0x00, sizeof(srKeyWritePara));
	memset(&uszHex, 0x00, sizeof(uszHex));

	/* 將Terminal Master KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;
	srKeyWritePara.Info.KeySet = _TMK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TMK_KEYINDEX_NCCC_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;
	srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_KPK;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_PLAINTEXT;

	/* 存15把key */
	for (i = 0; i < _KEY_TOTAL_COUNT_; i++)
	{
		/* 將ASCII的key轉成HEX */
		memset(szKeyData, 0x00, sizeof(szKeyData));
		memcpy(szKeyData, (char*)&TMK_KEY_TABLE[i], strlen((char*)&TMK_KEY_TABLE[i]));
		usKeyLength = strlen((char*)&TMK_KEY_TABLE[i]) / 2;

		inFunc_ASCII_to_BCD(uszHex, szKeyData, usKeyLength);
		srKeyWritePara.Value.pKeyData = uszHex;
		srKeyWritePara.Value.KeyLength = usKeyLength;

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

		if (inKMS_Write(&srKeyWritePara) != VS_SUCCESS)
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "第%d key寫入失敗.", i + 1);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
			inDISP_Wait(3000);
		}
		else
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "第%d key寫入成功.", i + 1);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
		}

		/* 寫入後換下一個位置 */
		srKeyWritePara.Info.KeyIndex++;
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_Test_TerminalMasterKey END()！");
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("------------------------------------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
 Function	:inNCCC_TMK_Write_TerminalMasterKey_By_KeyCard
 Date&Time	:2017/11/14 下午 5:57
 Describe	:寫入Production key
 *para.Version:			Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:		key set(一個key set只能有一種key type，EX:只能3DES or RSA)
 *para.Info.KeyIndex:		key index
 *para.Info.KeyType:		key之後會用在哪一種加密方法(EX:3DES or RSA......等)
 *para.Info.KeyVersion:		使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:	若有多個屬性，用"or"把bit on起來（KMS2_KEYATTRIBUTE_KPK表示是用來加密其他key的key）
 *para..Protection.Mode:	key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_PLAINTEXT 表示明碼寫入）
*/
int inNCCC_TMK_Write_TerminalMasterKey_By_KeyCard(int inKeyCnt, char *szKeyDataAscii)
{
	int			i = 0;
	int			inCnt = 0;
	char			szDebugMsg[100 + 1];
	char			szDispMsg[30 + 1];
	char			szKeyData[48 + 1];
	unsigned char		uszHex[24 + 1];		/* 3DES的Key長度最長24byte，若只輸入16byte則k1,k2,k3中，k3 = k1 */
	unsigned short		usKeyLength;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_TerminalMasterKey_By_KeyCard START()！");
		inDISP_LogPrintf(szDebugMsg);
	}

	memset(&srKeyWritePara, 0x00, sizeof(srKeyWritePara));
	memset(&uszHex, 0x00, sizeof(uszHex));

	/* 將Terminal Master KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;
	srKeyWritePara.Info.KeySet = _TMK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TMK_KEYINDEX_NCCC_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;
	srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_KPK;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_PLAINTEXT;

	/* 存15把key */
	for (i = 0; i < inKeyCnt; i++)
	{
		/* 將ASCII的key轉成HEX */
		memset(szKeyData, 0x00, sizeof(szKeyData));
		memcpy(szKeyData, &szKeyDataAscii[inCnt], 32);
		usKeyLength = strlen(szKeyData) / 2;

		inFunc_ASCII_to_BCD(uszHex, szKeyData, usKeyLength);
		srKeyWritePara.Value.pKeyData = uszHex;
		srKeyWritePara.Value.KeyLength = usKeyLength;

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

		if (inKMS_Write(&srKeyWritePara) != VS_SUCCESS)
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "第%d key寫入失敗.", i + 1);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
			inDISP_Wait(3000);
		}
		else
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "第%d key寫入成功.", i + 1);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
		}

		inCnt += strlen(szKeyData);
		/* 寫入後換下一個位置 */
		srKeyWritePara.Info.KeyIndex++;
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_TerminalMasterKey_By_KeyCard END()！");
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("------------------------------------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_Write_PINKey
Date&Time       :2016/2/16 上午 9:20
Describe        :
 *para.Version:				Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:			key set(一個key set只能有一種key type)
 *para.Info.KeyIndex:			key index
 *para.Info.KeyType:			key之後會用在哪一種加密方法
 *para.Info.KeyVersion:			使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:		若有多個屬性，用"or"把bit on起來
 *Para.Protection.CipherKeySet		KetProtectionKey的KeySet
 *Para.Protection.CipherKeyIndex	KetProtectionKey的KeyIndex
 *para.Protection.Mode:			key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_KPK_ECB 表示ECB寫入）
 *usTMKindex:				用第幾把Master Key
 *usPINKeyLen:				PINKey HEX長度
 *szPINKeyAscii				PINKey Ascii形式
 *szKeyCheckValueAscii			KeyCheckValue Ascii形式
*/
int inNCCC_TMK_Write_PINKey(unsigned short usTMKindex, unsigned short usPINKeyLen, char *szPINKeyAscii, char* szKeyCheckValueAscii)
{
	char			szDebugMsg[100 + 1];
	char			szAscii[64 + 1];
	unsigned char		uszPINKeyHex[24 + 1];		/* 3DES最長24BYTE */
	unsigned char		uszKeyCheckValueHex[4 + 1];	/* KeyCheckValue 3byte 求偶數加1byte */
	unsigned short		usKeyCheckValueLength;
	unsigned short		usReturnValue;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;


	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_PINKey START()！");
		inDISP_LogPrintf(szDebugMsg);
	}

	memset(&srKeyWritePara, 0x00, sizeof(CTOS_KMS2KEYWRITE_PARA));
	memset(&uszPINKeyHex, 0x00, sizeof(uszPINKeyHex));
	memset(&usKeyCheckValueLength, 0x00, sizeof(usKeyCheckValueLength));
	memset(&uszKeyCheckValueHex, 0x00, sizeof(uszKeyCheckValueHex));

	/* 可以不帶KeyCheckValue */
	if (strlen(szKeyCheckValueAscii) > 0)
	{
		/* Key Check Value Length*/
		usKeyCheckValueLength = strlen(szKeyCheckValueAscii) / 2;							/* CheckValue 是key來加密全0 Block之後密文的前6位字母數字(3byte hex)，所以hardcode */
		/* 將key check value轉成HEX格式 */
		inFunc_ASCII_to_BCD(uszKeyCheckValueHex, szKeyCheckValueAscii, usKeyCheckValueLength);
	}

	/* 將key轉成HEX格式 */
	inFunc_ASCII_to_BCD(uszPINKeyHex, szPINKeyAscii, usPINKeyLen);

	/* 將PIN KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;
	srKeyWritePara.Info.KeySet = _TWK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TWK_KEYINDEX_NCCC_PIN_ONLINE_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;
	srKeyWritePara.Info.KeyAttribute =  KMS2_KEYATTRIBUTE_PIN | KMS2_KEYATTRIBUTE_ENCRYPT;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_KPK_ECB;
	srKeyWritePara.Protection.CipherKeySet = _TMK_KEYSET_NCCC_;
	srKeyWritePara.Protection.CipherKeyIndex = _TMK_KEYINDEX_NCCC_ + (usTMKindex - 1);	/* 0x0000放第一把key，0x0001放第二把key，所以減一 */
	srKeyWritePara.Value.KeyLength = usPINKeyLen;
	srKeyWritePara.Value.pKeyData = uszPINKeyHex;

	if (usKeyCheckValueLength > 0)
	{
		srKeyWritePara.Verification.Method = KMS2_KEYVERIFICATIONMETHOD_DEFAULT;
		srKeyWritePara.Verification.KeyCheckValueLength = usKeyCheckValueLength;
		srKeyWritePara.Verification.pKeyCheckValue = uszKeyCheckValueHex;
	}

	/* Write PIN Key*/
	usReturnValue = inKMS_Write(&srKeyWritePara);

	if (usReturnValue != VS_SUCCESS)
	{
		/* 失敗 */
		if (ginDebug == VS_TRUE)
		{
			/* 被加密的working key */
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, srKeyWritePara.Value.pKeyData, srKeyWritePara.Value.KeyLength);
			inDISP_LogPrintf("Failed Encrypted Key :%d",usReturnValue);
			sprintf(szDebugMsg, "%s", szAscii);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_TMK_Write_PINKey END()！");
			inDISP_LogPrintf(szDebugMsg);
			inDISP_LogPrintf("------------------------------------------------------------------");
		}

		return (VS_ERROR);
	}
	else
	{
        
                    inNCCC_GetPkeyInfo();
        
		/* 成功 */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_TMK_Write_PINKey END()！");
			inDISP_LogPrintf(szDebugMsg);
			inDISP_LogPrintf("------------------------------------------------------------------");
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_TMK_Write_MACKey
Date&Time       :2016/2/16 上午 9:58
Describe        :
 *para.Version:				Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:			key set(一個key set只能有一種key type)
 *para.Info.KeyIndex:			key index
 *para.Info.KeyType:			key之後會用在哪一種加密方法
 *para.Info.KeyVersion:			使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:		若有多個屬性，用"or"把bit on起來
 *Para.Protection.CipherKeySet		KetProtectionKey的KeySet
 *Para.Protection.CipherKeyIndex	KetProtectionKey的KeyIndex
 *para.Protection.Mode:			key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_KPK_ECB 表示ECB寫入）
 *usTMKindex:				用第幾把Master Key
 *usMACKeyLen:				MACKey MACKey HeX長度
 *szMACKeyAscii				MACKey Ascii形式
 *szKeyCheckValueAscii			KeyCheckValue Ascii形式
*/
int inNCCC_TMK_Write_MACKey(unsigned short usTMKindex, unsigned short usMACKeyLen, char *szMACKeyAscii, char* szKeyCheckValueAscii)
{
	char			szDebugMsg[100 + 1];
	char			szAscii[64 + 1];
	unsigned char		uszMACKeyHex[24 + 1];		/* 3DES最長24BYTE */
	unsigned char		uszKeyCheckValueHex[4 + 1];	/* KeyCheckValue 3byte 求偶數加1byte */
	unsigned short		usKeyCheckValueLength;
	unsigned short		usReturnValue;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_MACKey START()！");
		inDISP_LogPrintf(szDebugMsg);
	}

	memset(&srKeyWritePara, 0x00, sizeof(CTOS_KMS2KEYWRITE_PARA));
	memset(&uszMACKeyHex, 0x00, sizeof(uszMACKeyHex));
	memset(&usKeyCheckValueLength, 0x00, sizeof(usKeyCheckValueLength));
	memset(uszKeyCheckValueHex, 0x00, sizeof(uszKeyCheckValueHex));

	if (strlen(szKeyCheckValueAscii) > 0)
	{
		/* Key Check Value Length*/
		usKeyCheckValueLength = strlen(szKeyCheckValueAscii) / 2;		/* CheckValue 是key來加密全0 Block之後密文的前6位字母數字(3byte hex)，所以hardcode */
		/* 將key check value轉成HEX格式 */
		inFunc_ASCII_to_BCD(uszKeyCheckValueHex, szKeyCheckValueAscii, usKeyCheckValueLength);
	}
	/* 將key轉成HEX格式 */
	inFunc_ASCII_to_BCD(uszMACKeyHex, szMACKeyAscii, usMACKeyLen);

	/* 將PIN KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;
	srKeyWritePara.Info.KeySet = _TWK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TWK_KEYINDEX_NCCC_MAC_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;
	srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_MAC | KMS2_KEYATTRIBUTE_ENCRYPT;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_KPK_ECB;
	srKeyWritePara.Protection.CipherKeySet = _TMK_KEYSET_NCCC_;
	srKeyWritePara.Protection.CipherKeyIndex = _TMK_KEYINDEX_NCCC_ + (usTMKindex - 1);	/* 0x0000放第一把key，0x0001放第二把key，所以減一 */
	srKeyWritePara.Value.KeyLength = usMACKeyLen;
	srKeyWritePara.Value.pKeyData = uszMACKeyHex;

	if (usKeyCheckValueLength > 0)
	{
		srKeyWritePara.Verification.Method = KMS2_KEYVERIFICATIONMETHOD_DEFAULT;
		srKeyWritePara.Verification.KeyCheckValueLength = usKeyCheckValueLength;
		srKeyWritePara.Verification.pKeyCheckValue = uszKeyCheckValueHex;
	}

	/* Write MAC Key*/
	usReturnValue = inKMS_Write(&srKeyWritePara);

	if (usReturnValue != VS_SUCCESS)
	{
		/* 失敗 */
		if (ginDebug == VS_TRUE)
		{
			/* 被加密的working key */
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, srKeyWritePara.Value.pKeyData, srKeyWritePara.Value.KeyLength);
			inDISP_LogPrintf("Failed Encrypted Key :");
			sprintf(szDebugMsg, "%s", szAscii);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_TMK_Write_MACKey END()！");
			inDISP_LogPrintf(szDebugMsg);
			inDISP_LogPrintf("------------------------------------------------------------------");
		}

		return (VS_ERROR);
	}
	else
	{
		/* 成功 */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_TMK_Write_MACKey END()！");
			inDISP_LogPrintf(szDebugMsg);
			inDISP_LogPrintf("------------------------------------------------------------------");
		}

		return (VS_SUCCESS);
	}

}

/*
 Function	:inNCCC_TMK_Write_ESCKey
 Date&Time	:2016/4/21 上午 9:54
 Describe	:寫ESCKey1的function
 *para.Version:			Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:		key set(一個key set只能有一種key type，EX:只能3DES or RSA)
 *para.Info.KeyIndex:		key index
 *para.Info.KeyType:		key之後會用在哪一種加密方法(EX:3DES or RSA......等)
 *para.Info.KeyVersion:		使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:	若有多個屬性，用"or"把bit on起來（KMS2_KEYATTRIBUTE_KPK表示是用來加密其他key的key）
 *para..Protection.Mode:	key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_PLAINTEXT 表示明碼寫入）
 *Key1 = TID(末5) + InviceNum(3)
 *Key2 = BatchNum(2) + 交易時間 hhmm(4) + 金額有效位數前兩碼，兩碼以下左補0(2)
	註:金額欄位取輸入F_04有效位數之前2碼。F_04 未上傳本欄位預設值=00
	例：	F_04 = 000000000000  若金額為0元，右靠左補0，取得金額=00
		F_04 = 000000000100  若金額為個位數1元，右靠左補0，取得金額=01
		F_04 = 000000001200  若金額為十位數12元，取金額前2碼=12
		F_04 = 000000012300  若金額為百位數123元，取金額前2碼=12
 * Encryption Key(szKeyFull) = Key1 + Key2 + Key1
 *
 *這裡比較特別的是key直接組成hex形式
 *例:填入的key值為139950140115214013995014
 *則原來可視的值為313339393530313430313135323134303133393935303134
*/
int inNCCC_TMK_Write_ESCKey(TRANSACTION_OBJECT *pobTran)
{
	int			inCnt = 0;		/* 記錄放到KEY的第幾位 */
	int			inCntAll = 0;		/* 記錄放到全部長度KEY的第幾位 */
	char			szDebugMsg[100 + 1];
	char			szTemplate[42 + 1];
	char			szKeyFull[24 + 1];	/* 放已組完全部長度的Key */
	char			szKeyTemp[16 + 1];
	unsigned char		uszHex[24 + 1];		/* 3DES的Key長度最長24byte，若只輸入16byte則k1,k2,k3中，k3 = k1 */
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_ESCKey START()！");
		inDISP_LogPrintf(szDebugMsg);
	}

	memset(szKeyFull, 0x00, sizeof(szKeyFull));


	/* Key 1 =================================================== */
	/* 初始化 */
	inCnt = 0;
	memset(szKeyTemp, 0x00, sizeof(szKeyTemp));
	/* 1.TID (ESC Host) */
	/* 切換到ESC的TID */
/* 有重覆，因為先不要換HOST，先拿掉*/
//	if (inNCCC_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
//	{
//		return (VS_ERROR);
//	}

	/* 卡號前4碼 */
	memcpy(&szKeyTemp[inCnt], &pobTran->srBRec.szPAN[0], 4);
	inCnt += 4;
	inDISP_LogPrintfWithFlag(" ESCKEY PAN[%s]", pobTran->srBRec.szPAN);

	/* 1299 */
	memcpy(&szKeyTemp[inCnt], "1299", 4);
	inCnt += 4;
	inDISP_LogPrintfWithFlag(" ESCKEY HARD CODE[%s]", "1299");

        /* TID */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	memcpy(&szKeyTemp[inCnt], szTemplate, 8);
	inCnt += 8;

	inDISP_LogPrintfWithFlag(" ESCKEY TID [%s]", szTemplate);

	inDISP_LogPrintfWithFlag(" ESCKEY BUF [%s]", szKeyTemp);

	/* 拷貝KEY值 */
	memcpy(&szKeyFull[inCntAll], szKeyTemp, 16);
	inCntAll += 16;

	/* 回覆原Host */
	if( inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inDISP_DispLogAndWriteFlie(" inNCCC_TMK_Write_ESCKey Load HDT[%d] *Error* Line[%d]",pobTran->srBRec.inHDTIndex,__LINE__);
	}

	memset(&srKeyWritePara, 0x00, sizeof(srKeyWritePara));
	memset(&uszHex, 0x00, sizeof(uszHex));

	/* 將所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;
	srKeyWritePara.Info.KeySet = _TWK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TWK_KEYINDEX_NCCC_ESC_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;
	srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_ENCRYPT;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_PLAINTEXT;
	srKeyWritePara.Value.pKeyData = (unsigned char*)szKeyFull;
	srKeyWritePara.Value.KeyLength = strlen(szKeyFull);

	if (inKMS_Write(&srKeyWritePara) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_ESCKey END()！");
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("------------------------------------------------------------------");
	}


	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_ESC_3DES_Encrypt
Date&Time       :2016/4/21 上午 11:51
Describe        :以3DES ECB模式加密
 *para.Version:				Structure Format Version，可填入0x00或0x01
 *para.Protection.CipherKeySet		用來加密的KeySet
 *para.Protection.CipherKeyIndex	用來加密的KeyIndex
 *para.Protection.CipherMethod		加密的method是CBC
 *para.Protection.SK_Length		SK_Length = 0 表示不使用session key(需要用DUKPT)
 *para.ICV.Length			Initial Vector Length
 *para.ICV.pData			Initial Vector
 *para.Input.pData			要加密的資料
 *para.Output.pData			產生的MAC
 *inLength				剩下未處理的資料長度
 *inIndex				已處理的資料長度
 *szInitialVector			當次加密用的InitialVector
 *szPlaindata				被切成8bytes用來加密的資料塊
*/
int inNCCC_TMK_ESC_3DES_Encrypt(char* szInPlaindata, int inInPlaindataLen, char *szResult)
{
	int				inRetVal;
//	char				szAscii[64 + 1];
//	char				szDebugMsg[100 + 1];
//	char				szAscii[4096 + 1];  /*   電簽要全加，所以要放大欄位  [SAM] */
	char				szDebugMsg[4096 + 1];
	char *szAscii;
	long lnCountData,lnConut,lnAddr;
	CTOS_KMS2DATAENCRYPT_PARA	srDataEncryptPara;

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);

	
	/* DEUBG */
	lnCountData = (inInPlaindataLen*2)+1 ;
	inDISP_LogPrintfWithFlag("  inInPlaindataLen[%d] CountData[%ld]",inInPlaindataLen, lnCountData);
	szAscii = malloc(lnCountData);
	
	memset(szAscii, 0x00, lnCountData);

	inFunc_BCD_to_ASCII(szAscii, (unsigned char *)szInPlaindata, inInPlaindataLen);
	
	lnConut = lnCountData / 4000;
	lnAddr = 0;
	
	while(lnConut)
	{	
		memset(szDebugMsg,0x00,sizeof(szDebugMsg));
		memcpy(szDebugMsg,&szAscii[lnAddr], 4000);
		inDISP_LogPrintfWithFlag("Cnt[%d][%s]", lnConut, szDebugMsg);
		lnConut --;
		
		lnCountData -= 4000;
		lnAddr += 4000;
	}
	
	inDISP_LogPrintfWithFlag("Cnt[%d][%s]",lnConut , &szAscii[lnAddr]);	
	/* DEUBG  END */	

	memset(&srDataEncryptPara, 0x00, sizeof(CTOS_KMS2DATAENCRYPT_PARA));
	srDataEncryptPara.Version = 0x01;
	srDataEncryptPara.Protection.CipherKeySet = _TWK_KEYSET_NCCC_;
	srDataEncryptPara.Protection.CipherKeyIndex = _TWK_KEYINDEX_NCCC_ESC_;
	srDataEncryptPara.Protection.CipherMethod = KMS2_DATAENCRYPTCIPHERMETHOD_ECB;
	srDataEncryptPara.Protection.SK_Length = 0;

	srDataEncryptPara.Input.Length = inInPlaindataLen;
	srDataEncryptPara.Input.pData = (unsigned char*)szInPlaindata;
	srDataEncryptPara.Output.pData = (unsigned char*)szResult;
	inRetVal = inKMS_DataEncrypt(&srDataEncryptPara);

	free(szAscii);
	
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_LogPrintfWithFlag("  ESC_3DES_Encrypt Failed");
		
		return (VS_ERROR);
	}
	else
	{
		inDISP_LogPrintfWithFlag("  ESC_3DES_Encrypt Success");
	}


	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
	return (VS_SUCCESS);
}


int inNCCC_TMK_ESC_3DES_Decrypt(char* szInPlaindata, int inInPlaindataLen, char *szResult)
{
	int				inRetVal;
//	char				szAscii[64 + 1];
//	char				szDebugMsg[100 + 1];
	char				szAscii[4096 + 1];  /*   電簽要全加，所以要放大欄位  [SAM] */
	char				szDebugMsg[4096 + 1];
	CTOS_KMS2DATAENCRYPT_PARA	srDataEncryptPara;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_ESC_3DES_Encrypt START(%d)",inInPlaindataLen);
		inDISP_LogPrintf(szDebugMsg);
	}

	if (ginDebug == VS_TRUE)
	{
		char szDebugMsg[5000 + 1];

		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char *)szInPlaindata, inInPlaindataLen);
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", szAscii);
		inDISP_LogPrintf(szDebugMsg);
	}

	memset(&srDataEncryptPara, 0x00, sizeof(CTOS_KMS2DATAENCRYPT_PARA));
	srDataEncryptPara.Version = 0x01;
	srDataEncryptPara.Protection.CipherKeySet = _TWK_KEYSET_NCCC_;
	srDataEncryptPara.Protection.CipherKeyIndex = _TWK_KEYINDEX_NCCC_ESC_;
	srDataEncryptPara.Protection.CipherMethod = KMS2_DATAENCRYPTCIPHERMETHOD_ECB;
	srDataEncryptPara.Protection.SK_Length = 0;

	srDataEncryptPara.Input.Length = inInPlaindataLen;
	srDataEncryptPara.Input.pData = (unsigned char*)szInPlaindata;
	srDataEncryptPara.Output.pData = (unsigned char*)szResult;
	inRetVal = inKMS_DataEncrypt(&srDataEncryptPara);

	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ESC_3DES_Encrypt Failed");
			inDISP_LogPrintf(szDebugMsg);
		}

		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ESC_3DES_Encrypt Success");
			inDISP_LogPrintf(szDebugMsg);
		}
	}


	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_ESC_3DES_Encrypt END()！");
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("------------------------------------------------------------------");
	}

	return (VS_SUCCESS);
}



/*
Function        :inNCCC_TMK_CalculatePINBlock
Date&Time       :2016/2/16 下午 1:35
Describe        :
 *para.Version:				Structure Format Version，可填入0x00或0x01
 *Para.PIN_Info.BlockType:		Block的type;
 *Para.PIN_Info.PINDigitMinLength:	PIN接受最短長度 只能4到12
 *Para.PIN_Info.PINDigitMaxLength:	PIN接受最長長度 只能4到12
 *Para.Protection.CipherKeySet		KetProtectionKey的KeySet
 *Para.Protection.CipherKeyIndex	KetProtectionKey的KeyIndex
 *para.Protection.Mode:			key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_KPK_ECB 表示ECB寫入）
 *Para.AdditionalData.InLength:		This field is used as the length of PAN if BlockType is KMS2_PINBLOCKTYPE_ANSI_X9_8_ISO_0.
 *Para.AdditionalData.pInData:		This field is used as the PAN data if BlockType is KMS2_PINBLOCKTYPE_ANSI_X9_8_ISO_0.Note that for PAN data, it shall also contain the last check digit.
 *Para.PINOutput.EncryptedBlockLength:	加密後的block長度（一般為8個byte）
 *Para.PINOutput.pEncryptedBlock:	output，加密後的PINBlock
 *Para.Control.Timeout:			時間到沒輸入PIN回傳d_KMS2_GET_PIN_TIMEOUT
 *Para.Control.AsteriskPositionX:	米字出現在第幾行
 *Para.Control.AsteriskPositionY:	米字出現在第幾列
 *Para.Control.NULLPIN:			可不可以輸入空值當PIN
 *Para.Control.piTestCancel:		可加入輸入PIN時額外的call back function
*/
int inNCCC_TMK_CalculatePINBlock(TRANSACTION_OBJECT* pobTran, char *szOutputPINBlock)
{
	//char			szASCII[42 + 1] = {0};
	char			szTimeOut[3 + 1] = {0};
	char			szAmountMsg[_DISP_MSG_SIZE_ + 1] = {0};
	char			szDemoMode[2 + 1] = {0};
	unsigned int		uiTimeOut = 0;
	unsigned short		usReturnValue = 0;
	CTOS_KMS2PINGET_PARA	srPINGetPara;
	char		szPIN[16 + 1];

	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CUP_GET_PASSWORD_IN_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 顯示金額 */
	memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
	if ((pobTran->srBRec.uszVOIDBit == VS_TRUE &&
	    (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	||
	     pobTran->srBRec.inCode == _REFUND_			||
	     pobTran->srBRec.inCode == _INST_REFUND_		||
	     pobTran->srBRec.inCode == _REDEEM_REFUND_		||
	     pobTran->srBRec.inCode == _CUP_REFUND_		||
	     pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
	{
		sprintf(szAmountMsg, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
	}
	else
	{
		sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
	}
	inFunc_Amount_Comma(szAmountMsg, "NT$", ' ', _SIGNED_NONE_,  15, _PAD_LEFT_FILL_RIGHT_);
	inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 7);

	memset(&srPINGetPara, 0x00, sizeof(CTOS_KMS2PINGET_PARA));
	memset(szPIN, 0x00, sizeof(szPIN));

	/* 如果沒設定TimeOut，就用EDC.dat內的TimeOut */
	if (uiTimeOut <= 0)
	{
		memset(szTimeOut, 0x00, sizeof(szTimeOut));
		inGetCUPOnlinePINEntryTimeout(szTimeOut);
		uiTimeOut = atoi(szTimeOut);
	}

	srPINGetPara.Version = 0x01;
	srPINGetPara.PIN_Info.BlockType = KMS2_PINBLOCKTYPE_ANSI_X9_8_ISO_0;
	srPINGetPara.PIN_Info.PINDigitMinLength = 4;
	srPINGetPara.PIN_Info.PINDigitMaxLength = 12;

	srPINGetPara.Protection.CipherKeySet = _TWK_KEYSET_NCCC_;
	srPINGetPara.Protection.CipherKeyIndex = _TWK_KEYINDEX_NCCC_PIN_ONLINE_;
	srPINGetPara.Protection.CipherMethod = KMS2_PINCIPHERMETHOD_ECB;

	srPINGetPara.AdditionalData.InLength = strlen(pobTran->srBRec.szPAN);
	srPINGetPara.AdditionalData.pInData = (unsigned char*)pobTran->srBRec.szPAN;

	srPINGetPara.PINOutput.EncryptedBlockLength = 8;
	srPINGetPara.PINOutput.pEncryptedBlock = (unsigned char*)szPIN;
	srPINGetPara.Control.Timeout = uiTimeOut;

/* 因UPT底層會自己建出密碼介面，所以需要重新設定 * 號密碼的位置  20190523  [SAM] */
#if _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_
	srPINGetPara.Control.AsteriskPositionX = 2;
	srPINGetPara.Control.AsteriskPositionY = 4;
#else	
	srPINGetPara.Control.AsteriskPositionX = 2;
	srPINGetPara.Control.AsteriskPositionY = 15;
#endif
	srPINGetPara.Control.NULLPIN = TRUE;
	srPINGetPara.Control.piTestCancel = NULL;

	/* 嗶三聲 */
	inDISP_BEEP(3, 500);

/* 因UPT底層會自己建出密碼介面，所以需要重新清畫面  20190523  [SAM] */
#if _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_
	inDISP_Clear_Line(_LINE_8_1_, _LINE_8_8_);
#endif
	/* Get PIN */
	usReturnValue = CTOS_KMS2PINGet(&srPINGetPara);
	if(memcmp(szPIN,"\x00\x00\x00\x00\x00\x00\x00\x00",8))
		inFunc_BCD_to_ASCII(szOutputPINBlock, (unsigned char*)szPIN ,8);

	inDISP_LogPrintfWithFlag("PINBlock: %s", szOutputPINBlock);

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] DEMO END -----",__FILE__, __FUNCTION__, __LINE__);
		return (VS_SUCCESS);
	}
	else
	{
		if (usReturnValue == d_KMS2_GET_PIN_TIMEOUT)
		{
			/* TimeOut */
			inDISP_DispLogAndWriteFlie("  Get PINBlock TimeOut. 代碼：0x%04X Line[%d]", usReturnValue, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_TIMEOUT);
		}
		else if ((usReturnValue == d_KMS2_GET_PIN_NULL_PIN && srPINGetPara.Control.NULLPIN == TRUE))
		{
/* 因UPT底層會自己建出密碼介面，所以需要重新清畫面，不用重新顯示金額  20190523  [SAM] */
#if _MACHINE_TYPE_ != _CASTLE_TYPE_UPT1000_
			/* 顯示金額 */
			memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
			if ((pobTran->srBRec.uszVOIDBit == VS_TRUE &&
			    (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	||
			     pobTran->srBRec.inCode == _REFUND_			||
			     pobTran->srBRec.inCode == _INST_REFUND_		||
			     pobTran->srBRec.inCode == _REDEEM_REFUND_		||
			     pobTran->srBRec.inCode == _CUP_REFUND_		||
			     pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szAmountMsg, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
			}
			else
			{
				sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
			}
			inFunc_Amount_Comma(szAmountMsg, "NT$", ' ', _SIGNED_NONE_,  15, _PAD_LEFT_FILL_RIGHT_);
			inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 7);
#endif
			/* 成功 */
			inDISP_DispLogAndWriteFlie("Get PINBlock Bypass.");
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

			return (VS_SUCCESS);
		}
		else if (usReturnValue == d_OK)
		{
/* 因UPT底層會自己建出密碼介面，所以需要重新清畫面，不用重新顯示金額  20190523  [SAM] */
#if _MACHINE_TYPE_ != _CASTLE_TYPE_UPT1000_
			/* 顯示金額 */
			memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
			if ((pobTran->srBRec.uszVOIDBit == VS_TRUE &&
			    (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	||
			     pobTran->srBRec.inCode == _REFUND_			||
			     pobTran->srBRec.inCode == _INST_REFUND_		||
			     pobTran->srBRec.inCode == _REDEEM_REFUND_		||
			     pobTran->srBRec.inCode == _CUP_REFUND_		||
			     pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szAmountMsg, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
			}
			else
			{
				sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
			}
			inFunc_Amount_Comma(szAmountMsg, "NT$", ' ', _SIGNED_NONE_,  15, _PAD_LEFT_FILL_RIGHT_);
			inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 7);
#endif
			/* 成功 */
			inDISP_DispLogAndWriteFlie("Get PINBlock Success.");
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);

			return (VS_SUCCESS);
		}
		else if (usReturnValue == d_KMS2_KEY_NOT_EXIST)
		{
			inDISP_DispLogAndWriteFlie("  Get PINBlock Failed. 代碼：0x%04X Line[%d]", usReturnValue, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}else
		{
			inDISP_DispLogAndWriteFlie("  Get PINBlock Failed. 代碼：0x%04X Line[%d]", usReturnValue, __LINE__);
			inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
			return (VS_ERROR);
		}
	}
}

/*
Function        :inNCCC_TMK_CalculateMac
Date&Time       :2016/2/16 下午 4:46
Describe        :根據CBC的加密方式，需要將加密的資料分塊的第一塊與initial vector做加密，所得值再當作下一區塊的initial vector
 *para.Version:				Structure Format Version，可填入0x00或0x01
 *para.Protection.CipherKeySet		用來加密的KeySet
 *para.Protection.CipherKeyIndex	用來加密的KeyIndex
 *para.Protection.CipherMethod		加密的method是CBC
 *para.Protection.SK_Length		SK_Length = 0 表示不使用session key(需要用DUKPT)
 *para.ICV.Length			Initial Vector Length
 *para.ICV.pData			Initial Vector
 *para.Input.pData			要加密的資料
 *para.Output.pData			產生的MAC
 *inLength				剩下未處理的資料長度
 *inIndex				已處理的資料長度
 *szInitialVector			當次加密用的InitialVector
 *szPlaindata				被切成8bytes用來加密的資料塊
*/
int inNCCC_TMK_CalculateMac(char* szInPlaindata, char *szMACdata)
{
	int			i;
	int			inLength;
	int			inIndex;
	int			inDebugLen;
	char			szInitialVector[8 + 1];
	char			szPlaindata[8 + 1];
	char			szDebugMsg[100 + 1];
	char			szASCII[42 + 1];
	unsigned short		usReturnValue;
	CTOS_KMS2MAC_PARA	srMACpara;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_CalculateMac START()！");
		inDISP_LogPrintf(szDebugMsg);
	}

	if (ginDebug == VS_TRUE)
	{
		inDebugLen = strlen(szInPlaindata);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "GenMACDATA: lenth: %d", inDebugLen);
		inDISP_LogPrintf(szDebugMsg);

		/* ADPU command 太長 */
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		for (i = 0; inDebugLen > 0; i++)
		{
			if (inDebugLen > 40)
			{
				memcpy(szDebugMsg, &szInPlaindata[i * 40], 40);
				inDISP_LogPrintf(szDebugMsg);
				memset(szDebugMsg, 0x00, 40);
				inDebugLen -= 40;
			}
			else
			{
				memcpy(szDebugMsg, &szInPlaindata[i * 40], inDebugLen);
				inDISP_LogPrintf(szDebugMsg);
				memset(szDebugMsg, 0x00, inDebugLen);
				inDebugLen -= inDebugLen;
			}

		}
	}

	memset(szInitialVector, 0x00, sizeof(szInitialVector));
	memset(szPlaindata, 0x00, sizeof(szPlaindata));
	memset(&srMACpara, 0x00, sizeof(CTOS_KMS2MAC_PARA));

	/*未處理資料長度*/
	inLength = strlen(szInPlaindata);
	/*已處理資料長度*/
	inIndex = 0;

	srMACpara.Version = 0x01;
	srMACpara.Protection.CipherKeySet = _TWK_KEYSET_NCCC_;
	srMACpara.Protection.CipherKeyIndex = _TWK_KEYINDEX_NCCC_MAC_;
	srMACpara.Protection.CipherMethod = KMS2_MACMETHOD_CBC;
	srMACpara.Protection.SK_Length = 0;
	srMACpara.ICV.Length = 8;
	srMACpara.ICV.pData = (unsigned char*)szInitialVector;

	srMACpara.Input.pData = (unsigned char*)szPlaindata;
	srMACpara.Output.pData = (unsigned char*)szMACdata;

	/* 只要還有沒處理的資料，就繼續做 */
	while (inLength > 0)
	{
		/* 最後一次長度不足8時，補0x00至8位，所以最後一次還是抓8個bytes */
		memcpy(szPlaindata, &szInPlaindata[inIndex], 8);
		srMACpara.Input.Length = strlen(szPlaindata);

		/* calculate MAC*/
		usReturnValue = CTOS_KMS2MAC(&srMACpara);

		inIndex += strlen(szPlaindata);
		inLength -= strlen(szPlaindata);

		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("------------------------------------------------------------------");

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(szASCII, (unsigned char*)szInitialVector, 8);
			sprintf(szDebugMsg, "InitialVector: %s", szASCII);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PlainDATA: %s", szPlaindata);
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PlainDATALength: %d", strlen(szPlaindata));
			inDISP_LogPrintf(szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(szASCII, (unsigned char*)szMACdata, 8);
			sprintf(szDebugMsg, "MACDATA: %s", szASCII);
			inDISP_LogPrintf(szDebugMsg);

			inDISP_LogPrintf("------------------------------------------------------------------");
		}

		if (usReturnValue != d_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Calculate MAC failed ret = 0x%04X", usReturnValue);
				inDISP_LogPrintf(szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "inNCCC_TMK_CalculateMac END()！");
				inDISP_LogPrintf(szDebugMsg);
				inDISP_LogPrintf("------------------------------------------------------------------");
			}

			return (VS_ERROR);
		}
		else
		{
			/* 繼續做 */
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Calculate MAC Success");
				inDISP_LogPrintf(szDebugMsg);

			}
		}

		/* 每加密一次就把產生的值，當成新一次加密的Initial vector */
		memcpy(szInitialVector, szMACdata, 8);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szASCII, 0x00, sizeof(szASCII));
		inFunc_BCD_to_ASCII(szASCII, (unsigned char*)szMACdata, 8);
		sprintf(szDebugMsg, "MACDATA: %s", szASCII);
		inDISP_LogPrintf(szDebugMsg);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_CalculateMac END()！");
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("------------------------------------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_GetKeyInfo_LookUp_Default
Date&Time       :2017/3/28 下午 1:54
Describe        :查看現存15把MasterKey狀態
*usKeySet	:要調查的keyset
*usKeyIndex	:要調查的KeyIndex
*inCVLen	:Check Value Length	不用的話填0 Only used for KeyType 3DES/3DES-DUKPT/AES
*inHashAlgorithm:Hash 演算法		不用的話填0 Only used for KeyType RSA
*/
int inNCCC_TMK_GetKeyInfo_LookUp_Default()
{
	int				inRetVal;
	int				inCVLen;
	int				inHashAlgorithm;
	char				szDebugMsg[100 + 1];
	char				szAscii[100 + 1];
	char				szDispMsg[50 + 1];
	char				szTemplate[50 + 1];
	char				szKeyCheckValue[6 + 1];
	char				szTest;
	unsigned short			usKeySet;
	unsigned short			usKeyIndex;
	CTOS_KMS2KEYGETINFO_PARA	srPara;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	/* 預設Key 位置 */
	usKeySet = _TMK_KEYSET_NCCC_;
	usKeyIndex = _TMK_KEYINDEX_NCCC_;

	/* CHeckValueLen
	 * Only used for KeyType 3DES/3DES-DUKPT/AES
	 */
	inCVLen = 6;

	/* HashAlgorithm
	 * Only used for KeyType RSA
	 * SHA1		KMS2_KEYCERTIFICATEGENERATECIHERMETHOD_DEFAULT_WITH_SHA1 (0x00)
	 * SHA256	KMS2_KEYCERTIFICATEGENERATECIHERMETHOD_DEFAULT_WITH_SHA2 (0x01)
	 */
	inHashAlgorithm = 0x00;

	for (usKeyIndex  = _TMK_KEYINDEX_NCCC_;usKeyIndex < _TMK_KEYINDEX_NCCC_ + _KEY_TOTAL_COUNT_; usKeyIndex++)
	{
		/* 放入結構中 */
		memset(&srPara, 0x00, sizeof(CTOS_KMS2KEYGETINFO_PARA));
		srPara.Version = 0x01;
		srPara.Input.KeySet = usKeySet;
		srPara.Input.KeyIndex = usKeyIndex;
		srPara.Input.CVLen = inCVLen;				/* Only used for KeyType 3DES/3DES-DUKPT/AES */
		srPara.Input.HashAlgorithm = inHashAlgorithm;		/* Only used for KeyType RSA */
		srPara.Output.pCV = (unsigned char*)szKeyCheckValue;

		inRetVal = inKMS_GetKeyInfo(&srPara);

		if (inRetVal == VS_SUCCESS)
		{
			/* 成功 */

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			switch (srPara.Output.KeyType)
			{
				case KMS2_KEYTYPE_3DES :
					strcat(szTemplate, "3DES");
					break;
				case KMS2_KEYTYPE_3DES_DUKPT :
					strcat(szTemplate, "3DES_DUKPT");
					break;
				case KMS2_KEYTYPE_AES :
					strcat(szTemplate, "AES");
					break;
				case KMS2_KEYTYPE_RSA :
					strcat(szTemplate, "RSA");
					break;
				case KMS2_KEYTYPE_DES_DUKPT :
					strcat(szTemplate, "DES_DUKPT");
					break;
				default :
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Not found, value : %d", srPara.Output.KeyType);
					strcat(szTemplate, szDebugMsg);
					break;
			}

			sprintf(szDispMsg, "%s : %s", "Key Type", szTemplate);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_, _DISP_LEFT_);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_PIN)
				strcat(szTemplate, "PIN/");
			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_ENCRYPT)
				strcat(szTemplate, "ENC/");
			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_MAC)
				strcat(szTemplate, "MAC/");
			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_KPK)
				strcat(szTemplate, "KPK/");
			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_DECRYPT)
				strcat(szTemplate, "DEC/");
			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_KBPK)
				strcat(szTemplate, "KBPK/");
			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_SK_ENCRYPT)
				strcat(szTemplate, "SK ENC/");
			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_INTERMEDIATE)
				strcat(szTemplate, "INTER/");
			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_FREEZE_RSA_WRITE_KEY_BY_CERTIFICATE)
				strcat(szTemplate, "FR RSA CE/");
			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_FREEZE_RSA_ENCRYPT)
				strcat(szTemplate, "FR RSA ENC/");
			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_CONSIDER_INVALID_BIT_AS_VALID_FOR_KEY_VALUE_UNIQUE)
				strcat(szTemplate, "CON/");
			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_VALUE_UNIQUE)
				strcat(szTemplate, "UNI/");
			if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_PROTECTED)
				strcat(szTemplate, "PRO/");

			if (szTemplate[strlen(szTemplate) - 1] == '/')
				szTemplate[strlen(szTemplate) - 1] = 0x00;

			sprintf(szDispMsg, "%s : %s", "Key Attr", szTemplate);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "KeyLength : %u", srPara.Output.KeyLength);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, (unsigned char*)srPara.Output.pCV, inCVLen);
			sprintf(szDispMsg, "Key CV : %s", szAscii);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);

			szTest = 0x00;
			inNCCC_TMK_Check_Test_Key(srPara.Input.KeySet, srPara.Input.KeyIndex, &szTest);
			if (szTest == 'Y')
			{
				inDISP_ChineseFont_Color("這是測試Key", _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_RED_, _DISP_LEFT_);
			}

		}
		else if (inRetVal == d_KMS2_KEY_NOT_EXIST)
		{
			inDISP_ChineseFont_Color("該位置Key不存在", _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_RED_, _DISP_LEFT_);
		}
		else
		{
			/* 失敗 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		}

		uszKBD_GetKey(30);
	}

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_ProductionKey_Swap_To_Temp
Date&Time       :2017/1/13 下午 1:30
Describe        :將Production Key塞到後面的位置暫存
 */
int inNCCC_TMK_ProductionKey_Swap_To_Temp()
{
	int	i;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	/* 因為Swap一定要兩個位置都有Key，所以檢查沒有Key就塞入全0預設Key */
	for (i = 0; i < _KEY_TOTAL_COUNT_; i++)
	{
		if (inKMS_CheckKey(_TMK_KEYSET_NCCC_, _TMK_KEYINDEX_NCCC_ + i) != VS_SUCCESS)
		{
			inKMS_Write_NULL_Key(_TMK_KEYSET_NCCC_, _TMK_KEYINDEX_NCCC_ + i);
		}

	}

	for (i = 0; i < _KEY_TOTAL_COUNT_; i++)
	{
		if (inKMS_CheckKey(_TMK_TEMP_KEYSET_NCCC_, _TMK_TEMP_KEYINDEX_NCCC_ + i) != VS_SUCCESS)
		{
			inKMS_Write_NULL_Key(_TMK_TEMP_KEYSET_NCCC_, _TMK_TEMP_KEYINDEX_NCCC_ + i);
		}

	}

	for (i = 0; i < _KEY_TOTAL_COUNT_; i++)
	{
		if (inKMS_Key_Swap(_TMK_KEYSET_NCCC_, _TMK_KEYINDEX_NCCC_ + i, _TMK_TEMP_KEYSET_NCCC_, _TMK_TEMP_KEYINDEX_NCCC_ + i) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

	}

	inDISP_ChineseFont_Color("Key 交換完成", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_, _DISP_CENTER_);


	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_Check_Test_Key
Date&Time       :2017/1/13 上午 11:30
Describe        :確認是否是測試Key，是測試Key則szTest填'Y'，不是測試Key填'N'，沒有key則為空(0x00)
*/
int inNCCC_TMK_Check_Test_Key(unsigned short usKeySet, unsigned short usKeyIndex, char *szTest)
{
        int i;
        int inRetVal;
        int inCVLen = 6;
        int inHashAlgorithm = 0;
        char    szKeyCheckValue[6 + 1];
        char    szAscii[12 + 1];
        CTOS_KMS2KEYGETINFO_PARA	srPara;

        /* 放入結構中 */
        memset(szKeyCheckValue, 0x00, sizeof(szKeyCheckValue));
        memset(&srPara, 0x00, sizeof(CTOS_KMS2KEYGETINFO_PARA));
        srPara.Version = 0x01;
        srPara.Input.KeySet = usKeySet;
        srPara.Input.KeyIndex = usKeyIndex;
        srPara.Input.CVLen = inCVLen;				/* Only used for KeyType 3DES/3DES-DUKPT/AES */
        srPara.Input.HashAlgorithm = inHashAlgorithm;		/* Only used for KeyType RSA */
        srPara.Output.pCV = (unsigned char*)szKeyCheckValue;
        *szTest = 0x00;

        inRetVal = inKMS_GetKeyInfo(&srPara);

        if (inRetVal != VS_SUCCESS)
        {
                inDISP_LogPrintfWithFlag(" Get Key Info *Error* Line[%d] ", __LINE__);
                return (VS_ERROR);
        }

        memset(szAscii, 0x00, sizeof(szAscii));
        inFunc_BCD_to_ASCII(szAscii, srPara.Output.pCV, srPara.Input.CVLen);

        inDISP_LogPrintfWithFlag(" Get Key Check Value Len[%d] Data[%s] Line[%d] ",srPara.Input.CVLen, szAscii, __LINE__);
    
    
        for (i = 0; i < _KCV_TOTAL_COUNT_; i++)
        {
                if (memcmp(szAscii, TMK_KCV_TABLE[i].szKCV, strlen(szAscii)) == 0)
                {
                        *szTest = 'Y';
                        return (VS_SUCCESS);
                }
        }

        *szTest = 'N';

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_Test
Date&Time       :2016/2/16 下午 5:03
Describe        :以第十一把TMK來解PINkey和MAC Key
*/
int inNCCC_TMK_Test()
{
	char			szTemplate[42 + 1];
	char			szKeyCheckValue[16 + 1];
	//unsigned short		usTMKindex = 0x0001;
	//unsigned short		usTMKindex = 0x000B;
	//unsigned short		usKeyLen = 32;


	inKMS_Initial();
	memset(szTemplate,0x00,sizeof(szTemplate));
	inFunc_ASCII_to_BCD((BYTE *)szTemplate, "32303138313230363035313430333933",16);
	inFUBON_KEK_Write((BYTE *)szTemplate,16);
	
	memset(szTemplate,0x00,sizeof(szTemplate));
	inFunc_ASCII_to_BCD((BYTE *)szTemplate, "D2A389563ECCBD52EBB415CF5C016FDD",16);
	memset(szKeyCheckValue,0x00,sizeof(szKeyCheckValue));
	inFunc_ASCII_to_BCD((BYTE *)szKeyCheckValue, "7BBD51",3);
	
	inFUBON_TMK_Write(KMS2_KEYPROTECTIONMODE_KPK_ECB,(BYTE *)szTemplate,16,(BYTE *)szKeyCheckValue,3);

	/*{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "D2A389563ECCBD52EBB415CF5C016FDD", 32);
		memset(szKeyCheckValue, 0x00, sizeof(szKeyCheckValue));
		memcpy(szKeyCheckValue, "7BBD51", 6);
		inNCCC_TMK_Write_PINKey(usTMKindex, usKeyLen/2, szTemplate, szKeyCheckValue);
	}*/

	{
		inNCCC_TMK_Write_PINKey(1, 16, "A6F1DAE5EE33D5105A848C5F1BEFFE0D", "5286BD");
	}

	/*{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "3D96384729C1C60066CCE7AEEE590BB0", 32);
		memset(szKeyCheckValue, 0x00, sizeof(szKeyCheckValue));
		memcpy(szKeyCheckValue, "936A6B", 6);
		inNCCC_TMK_Write_PINKey(usTMKindex, usKeyLen/2, szTemplate, szKeyCheckValue);
	}*/

#if 0
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "F44719377CF40775EA9CCBB7524DB9E7", 32);
		memset(szKeyCheckValue, 0x00, sizeof(szKeyCheckValue));
		memcpy(szKeyCheckValue, "B8ABA8", 6);
		inNCCC_TMK_Write_MACKey(usTMKindex, usKeyLen/2, szTemplate, szKeyCheckValue);
	}

	{
		char	szPINBlock[8 + 1];
		memset(szPINBlock, 0x00, sizeof(szPINBlock));

		TRANSACTION_OBJECT pobTran;
		memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
		memcpy(&pobTran.srBRec.szPAN, "452001234567890301", strlen("452001234567890301"));

		inNCCC_TMK_CalculatePINBlock(&pobTran, szPINBlock);
	}

	{
		char	szPlainData[128 + 1];
		char	szMACData[128 + 1];

		memset(szPlainData, 0x00, sizeof(szPlainData));
		memset(szMACData, 0x00, sizeof(szMACData));

		memcpy(szPlainData,"401399402000376348129192026D1903110416522000000010000",strlen("401399402000376348129192026D1903110416522000000010000"));

		inNCCC_TMK_CalculateMac(szPlainData, szMACData);
	}
#endif

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_ESCKey_Test
Date&Time       :2016/4/21 上午 11:06
Describe        :
*/
int inNCCC_TMK_ESCKey_Test()
{
	int i;
	char	szTemplate[10000 + 1];
	char	szTestTemp[5000 + 1];
	char szKeyTemp[64], szCopyTemp[64];
	
	char	szTemplate1[10000 + 1];
	char	szEncData[5000 + 1];
	
	TRANSACTION_OBJECT	pobTran;

	memset(&pobTran, 0x00, sizeof(pobTran));

//	if (inFunc_GetHostNum(&pobTran) != VS_SUCCESS)
//	{
//		return (VS_ERROR);
//	}
//
//	if (inBATCH_FuncUserChoice_By_Sqlite(&pobTran) != VS_SUCCESS)
//	{
//		return (VS_ERROR);
//	}
//
//	if (inBATCH_FuncUserChoice_By_Sqlite(&pobTran) != VS_SUCCESS)
//	{
//		return (VS_ERROR);
//	}
	/* TID 13995512 */
	pobTran.srBRec.lnOrgInvNum = 001;
	pobTran.srBRec.lnBatchNum = 001;
	memcpy(pobTran.srBRec.szTime, "1515", 4);
	pobTran.srBRec.lnTxnAmount = 11;

	memcpy(pobTran.srBRec.szPAN, "4938170100001213" ,16);
	
	if (inNCCC_TMK_Write_ESCKey(&pobTran) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	
	memset(szEncData, 0x00, sizeof(szEncData));
	
	memcpy(szEncData,"1111222223333444", 16);
	
	inDISP_LogPrintfWithFlag(" Fist 1 Encrypt ");
	
	i = 16;
	memset(szTestTemp, 0x00, sizeof(szTestTemp));
	if (inNCCC_TMK_ESC_3DES_Encrypt(szEncData, i, szTestTemp) != VS_SUCCESS)
	{
			inDISP_LogPrintfWithFlag(" Encrypt Err Len[%d]", i);
	}
	
	inFunc_BCD_to_ASCII(szTemplate1, (unsigned char *)szTestTemp, 16);
	inDISP_LogPrintfWithFlag(szTemplate1);
	
	
	inDISP_LogPrintfWithFlag(" Fist 2 Encrypt ");
	memset(szTestTemp, 0x00, sizeof(szTestTemp));
	
	if (inNCCC_TMK_ESC_3DES_Encrypt(szEncData, i, szTestTemp) != VS_SUCCESS)
	{
			inDISP_LogPrintfWithFlag(" Encrypt Err Len[%d]", i);
	}
	inFunc_BCD_to_ASCII(szTemplate1, (unsigned char *)szTestTemp, 16);
	inDISP_LogPrintfWithFlag(szTemplate1);
	
	inDISP_LogPrintfWithFlag(" Fist 3 Encrypt ");
	int inCnt = 0;
	memset(szKeyTemp, 0x00, sizeof(szKeyTemp));
	/* 卡號前4碼 */
	memcpy(&szKeyTemp[inCnt], &pobTran.srBRec.szPAN[0], 4);
	inCnt += 4;
	/* 1299 */
	memcpy(&szKeyTemp[inCnt], "1299", 4);
	inCnt += 4;
        /* TID */
	memset(szCopyTemp, 0x00, sizeof(szCopyTemp));
	inGetTerminalID(szCopyTemp);
	memcpy(&szKeyTemp[inCnt], szCopyTemp, 8);
	inCnt += 8;

	inDISP_LogPrintfWithFlag(" ESCKEY BUF [%s]", szKeyTemp);
	inDISP_LogPrintfWithFlag(" Enc BUF [%s]", szEncData);
		
	inDISP_LogPrintfWithFlag(" Fist 4 Encrypt ");
	memset(szTestTemp, 0x00, sizeof(szTestTemp));
	
	TDES_2LenKey_ECB_Encrypt((BYTE *)szKeyTemp, (BYTE *)szEncData, 16, (BYTE *)szTestTemp);
	
	inFunc_BCD_to_ASCII(szTemplate1, (unsigned char *)szTestTemp, 16);
	inDISP_LogPrintfWithFlag(szTemplate1);
	
	
		inDISP_LogPrintfWithFlag(" Fist 5 Encrypt ");
	memset(szTemplate, 0xbf, sizeof(szTemplate));
	memset(szTestTemp, 0x00, sizeof(szTestTemp));
//	for(i = 0 ; i < 3000 ; i+=16)
	{
		inDISP_LogPrintfWithFlag(" Encrypt Lenth [%d]", i );
		
		if (inNCCC_TMK_ESC_3DES_Encrypt(szTemplate, 1000, szTestTemp) != VS_SUCCESS)
		{
			inDISP_LogPrintfWithFlag(" Encrypt Err Len[%d]", i);
		}
	}
	inFunc_BCD_to_ASCII(szTemplate1, (unsigned char *)szTestTemp, 1000);
	inDISP_LogPrintfWithFlag(szTemplate1);
	
		inDISP_LogPrintfWithFlag(" Fist 6 Encrypt ");
	
	memset(szTestTemp, 0x00, sizeof(szTestTemp));
	TDES_2LenKey_ECB_Encrypt((BYTE *)szKeyTemp, (BYTE *)szTemplate, 1000, (BYTE *)szTestTemp);
	
	inFunc_BCD_to_ASCII(szTemplate1, (unsigned char *)szTestTemp, 1000);
	inDISP_LogPrintfWithFlag(szTemplate1);
	
	
//	memset(szAscii, 0x00, sizeof(szAscii));
//	inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szTemplate, 8);
//	/* should be 6A6F31F73B9D0FD0*/
//	inDISP_LogPrintf(szAscii);

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_MFES_Test
Date&Time       :2017/1/3 上午 9:34
Describe        :莫名其妙MFES安全認證不過
*/
int inNCCC_TMK_MFES_Test()
{
	char			szTemplate[42 + 1];
	char			szDebugMsg[100 + 1];
	char			szPlainData[128 + 1];
	char			szMACData[128 + 1];
	unsigned short		usTMKindex = 0x0001;
	unsigned short		usKeyLen = 32;


	inKMS_Initial();
	inNCCC_TMK_Write_Test_TerminalMasterKey();

	for (usTMKindex = 1; usTMKindex <= 15; usTMKindex++)
	{
//		{
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			memcpy(szTemplate, "3D96384729C1C60066CCE7AEEE590BB0", 32);
//			inNCCC_TMK_Write_PINKey(usTMKindex, usKeyLen/2, szTemplate, szKeyCheckValue);
//		}

		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, "2A238A173E9EA6EB01F40E7AAE793B67", 32);
			inNCCC_TMK_Write_MACKey(usTMKindex, usKeyLen/2, szTemplate, "");
		}

//		{
//			char	szPINBlock[8 + 1];
//			memset(szPINBlock, 0x00, sizeof(szPINBlock));
//
//			TRANSACTION_OBJECT pobTran;
//			memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
//			memcpy(&pobTran.srBRec.szPAN, "452001234567890301", strlen("452001234567890301"));
//
//			inNCCC_TMK_CalculatePINBlock(&pobTran, szPINBlock);
//		}

		{
			memset(szPlainData, 0x00, sizeof(szPlainData));
			memset(szMACData, 0x00, sizeof(szMACData));

			memcpy(szPlainData,"631399550200;3560500100001218D18121011697684900000?1000000000100",strlen("631399550200;3560500100001218D18121011697684900000?1000000000100"));

			inNCCC_TMK_CalculateMac(szPlainData, szMACData);
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			inFunc_BCD_to_ASCII(szDebugMsg, (unsigned char*)szMACData, 8);
			sprintf(szDebugMsg, "%s", szDebugMsg);
			inDISP_LogPrintf(szDebugMsg);
			/* MAC值should be 78E1F67BC3CD2C17 */
		}
	}

	return (VS_SUCCESS);
}


/*
 Function	:inFUBON_KEK_Writey
 Date&Time	:2016/2/5 下午 2:39
 Describe	:寫測試MasterKey的function
 *para.Version:			Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:		key set(一個key set只能有一種key type，EX:只能3DES or RSA)
 *para.Info.KeyIndex:		key index
 *para.Info.KeyType:		key之後會用在哪一種加密方法(EX:3DES or RSA......等)
 *para.Info.KeyVersion:		使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:	若有多個屬性，用"or"把bit on起來（KMS2_KEYATTRIBUTE_KPK表示是用來加密其他key的key）
 *para..Protection.Mode:	key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_PLAINTEXT 表示明碼寫入）
*/
int inFUBON_KEK_Write(BYTE *szKeyData,unsigned short usKeyLength)
{
	int			i;
	char			szDebugMsg[100 + 1];
	char			szDispMsg[30 + 1];
	//char			szKeyData[48 + 1];
	//unsigned char		uszHex[24 + 1];		/* 3DES的Key長度最長24byte，若只輸入16byte則k1,k2,k3中，k3 = k1 */
	//unsigned short		usKeyLength;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;


	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inFUBON_KEK_Writey START()！");
		inDISP_LogPrintf(szDebugMsg);
	}

	memset(&srKeyWritePara, 0x00, sizeof(srKeyWritePara));
	//memset(&uszHex, 0x00, sizeof(uszHex));

	/* 將Terminal Master KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;
	srKeyWritePara.Info.KeySet = _TMK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TMK_KEYINDEX_NCCC_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;
	srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_KPK;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_PLAINTEXT;

	/* 存15把key */
	for (i = 0; i < _KEY_TOTAL_COUNT_; i++)
	{
		/* 將ASCII的key轉成HEX */
		//memset(szKeyData, 0x00, sizeof(szKeyData));
		//memcpy(szKeyData, szKeyData, usKeyLength * 2);

		//inFunc_ASCII_to_BCD(uszHex, (char *)szKeyData, usKeyLength);
		srKeyWritePara.Value.pKeyData = szKeyData;
		srKeyWritePara.Value.KeyLength = usKeyLength;

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

		if (inKMS_Write(&srKeyWritePara) != VS_SUCCESS)
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "第%d key寫入失敗.", i + 1);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
			//inDISP_Wait(3000);
			return(VS_ERROR);
		}
		/*else
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "第%d key寫入成功.", i + 1);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
		}*/

		/* 寫入後換下一個位置 */
		srKeyWritePara.Info.KeyIndex++;
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inFUBON_KEK_Writey END()！");
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("------------------------------------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
 Function	:inFUBON_TMK_Write
 Date&Time	:2016/2/5 下午 2:39
 Describe	:寫測試MasterKey的function
 *para.Version:			Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:		key set(一個key set只能有一種key type，EX:只能3DES or RSA)
 *para.Info.KeyIndex:		key index
 *para.Info.KeyType:		key之後會用在哪一種加密方法(EX:3DES or RSA......等)
 *para.Info.KeyVersion:		使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:	若有多個屬性，用"or"把bit on起來（KMS2_KEYATTRIBUTE_KPK表示是用來加密其他key的key）
 *para..Protection.Mode:	key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_PLAINTEXT 表示明碼寫入）
*/
int inFUBON_TMK_Write(char usProtectionMode,BYTE *szKeyData,unsigned short usKeyLength,BYTE *szKeyCheckValueData,unsigned short usKeyCheckValueLength)
{
	char			szDebugMsg[100 + 1];
	char			szDispMsg[30 + 1];
	//char			szKeyData[48 + 1];
	//unsigned char		uszHex[24 + 1];		/* 3DES的Key長度最長24byte，若只輸入16byte則k1,k2,k3中，k3 = k1 */
	//unsigned short		usKeyLength;
	//unsigned char		uszKeyCheckValueHex[4 + 1];	/* KeyCheckValue 3byte 求偶數加1byte */
	//unsigned short		usKeyCheckValueLength;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;

	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inFUBON_TMK_Write START()！");
		inDISP_LogPrintf(szDebugMsg);
	}

	memset(&srKeyWritePara, 0x00, sizeof(srKeyWritePara));
	//memset(&uszHex, 0x00, sizeof(uszHex));

	/* 將Terminal Master KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;
	srKeyWritePara.Info.KeySet = _TMK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TMK_KEYINDEX_NCCC_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;
	srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_KPK | KMS2_KEYATTRIBUTE_ENCRYPT;
	srKeyWritePara.Protection.Mode = usProtectionMode;
	srKeyWritePara.Protection.CipherKeySet = _TMK_KEYSET_NCCC_;
	srKeyWritePara.Protection.CipherKeyIndex = _TMK_KEYINDEX_NCCC_;	/* 0x0000放第一把key，0x0001放第二把key，所以減一 */


	/* 存15把key */
	{
		/* 將ASCII的key轉成HEX */
		//memset(szKeyData, 0x00, sizeof(szKeyData));
		//memcpy(szKeyData, (char*)&TMK_KEY_TABLE[i], strlen((char*)&TMK_KEY_TABLE[i]));
		//usKeyLength = strlen((char*)&TMK_KEY_TABLE[i]) / 2;

		//inFunc_ASCII_to_BCD(uszHex, (char *)szKeyData, usKeyLength);
		srKeyWritePara.Value.pKeyData = szKeyData;
		srKeyWritePara.Value.KeyLength = usKeyLength;

		if (usKeyCheckValueLength > 0)
		{
			//usKeyCheckValueLength = strlen((char *)szKeyCheckValueAscii) / 2;							/* CheckValue 是key來加密全0 Block之後密文的前6位字母數字(3byte hex)，所以hardcode */
			//inFunc_ASCII_to_BCD(uszKeyCheckValueHex, (char *)szKeyCheckValueAscii, usKeyCheckValueLength);
			srKeyWritePara.Verification.Method = KMS2_KEYVERIFICATIONMETHOD_DEFAULT;
			srKeyWritePara.Verification.KeyCheckValueLength = usKeyCheckValueLength;
			srKeyWritePara.Verification.pKeyCheckValue = szKeyCheckValueData;
		}

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

		if (inKMS_Write(&srKeyWritePara) != VS_SUCCESS)
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "第%d key寫入失敗.", 1);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
			inDISP_LogPrintfWithFlag("  Write MK Err Index");
			//inDISP_Wait(3000);
			return(VS_ERROR);
		}
		/*else
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "第%d key寫入成功.", 1);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
		}*/

		/* 寫入後換下一個位置 */
		//srKeyWritePara.Info.KeyIndex++;
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inFUBON_TMK_Write END()！");
		inDISP_LogPrintf(szDebugMsg);
		inDISP_LogPrintf("------------------------------------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        : inNCCC_TMK_GetKeyCheckValue
Date&Time   : 2021/2/24 下午 6:03
Describe        : 抓取對應的 CHECK KEY VALUE
*/
int inNCCC_TMK_GetKeyCheckValue(unsigned short usKeySet, unsigned short usKeyIndex, unsigned char *uszCheckValue)
{
        int inRetVal;
        int inCVLen = 6;
        int inHashAlgorithm = 0;
        char    szKeyCheckValue[24 + 1];
        char    szAscii[48 + 1];
        CTOS_KMS2KEYGETINFO_PARA	srPara;

        /* 放入結構中 */
        memset(szKeyCheckValue, 0x00, sizeof(szKeyCheckValue));
        memset(&srPara, 0x00, sizeof(CTOS_KMS2KEYGETINFO_PARA));
        srPara.Version = 0x01;
        srPara.Input.KeySet = usKeySet;
        srPara.Input.KeyIndex = usKeyIndex;
        srPara.Input.CVLen = inCVLen;				/* Only used for KeyType 3DES/3DES-DUKPT/AES */
        srPara.Input.HashAlgorithm = inHashAlgorithm;		/* Only used for KeyType RSA */
        srPara.Output.pCV = (unsigned char*)szKeyCheckValue;

        inRetVal = inKMS_GetKeyInfo(&srPara);

        if (inRetVal != VS_SUCCESS)
        {
                inDISP_LogPrintfWithFlag(" Get Key Info *Error* Line[%d] ", __LINE__);
                return (VS_ERROR);
        }

        memset(szAscii, 0x00, sizeof(szAscii));
        inFunc_BCD_to_ASCII(szAscii,  srPara.Output.pCV, srPara.Input.CVLen);
        memcpy(uszCheckValue, srPara.Output.pCV, srPara.Input.CVLen);
        inDISP_LogPrintfWithFlag(" Get Key Check Value Len[%d] Data[%s] Line[%d] ",srPara.Input.CVLen, szAscii, __LINE__);
    
        return (VS_SUCCESS);
}



int inNCCC_GetMkeyInfo()
{
        char szKeyCheckValue[24];
        CTOS_KMS2KEYGETINFO_PARA	srPara;
    
        memset(&srPara, 0x00, sizeof(CTOS_KMS2KEYGETINFO_PARA));
        srPara.Version = 0x01;
        srPara.Input.KeySet = _TMK_KEYSET_NCCC_;
        srPara.Input.KeyIndex = _TMK_KEYINDEX_NCCC_;
        srPara.Input.CVLen = 6;				/* Only used for KeyType 3DES/3DES-DUKPT/AES */
        srPara.Input.HashAlgorithm = 0x00;		/* Only used for KeyType RSA */
        srPara.Output.pCV = (unsigned char*)szKeyCheckValue;
        inNCCC_TMK_GetKeyCheckValue(srPara.Input.KeySet, srPara.Input.KeyIndex, guszMKCheckValue);   
        return VS_SUCCESS;
}

int inNCCC_GetPkeyInfo()
{
        char szKeyCheckValue[24];
        CTOS_KMS2KEYGETINFO_PARA	srPara;
    
        memset(&srPara, 0x00, sizeof(CTOS_KMS2KEYGETINFO_PARA));
        srPara.Version = 0x01;
        srPara.Input.KeySet = _TMK_KEYSET_NCCC_;
        srPara.Input.KeyIndex = _TWK_KEYINDEX_NCCC_PIN_ONLINE_;
        srPara.Input.CVLen = 6;				/* Only used for KeyType 3DES/3DES-DUKPT/AES */
        srPara.Input.HashAlgorithm = 0x00;		/* Only used for KeyType RSA */
        srPara.Output.pCV = (unsigned char*)szKeyCheckValue;
        
            
        inNCCC_TMK_GetKeyCheckValue(srPara.Input.KeySet, srPara.Input.KeyIndex, guszWKkCheckValue);   
        return VS_SUCCESS;
}


