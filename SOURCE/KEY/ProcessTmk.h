/*
 * File:   NCCCtmk.h
 * Author: bai
 *
 * Created on 2016年1月5日, 上午 9:25
 */

/* Test Key data */
//#define _MULTI_TMK_DATA_1_	"23C73D3D970E5768C83E250B4FC2162F"
#define _MULTI_TMK_DATA_1_	"32303138313230363035313430333933"
//#define _MULTI_TMK_DATA_1_	"0123456789ABCDEF0123456789ABCDEF"
#define _MULTI_TMK_DATA_2_	"67310167C75EDA4034401064B38A40AE"
#define _MULTI_TMK_DATA_3_	"83625BC297AE57DFA238B0D679EA32D9"
#define _MULTI_TMK_DATA_4_	"494C70A78357C201C73B9B4A73F77A1A"
#define _MULTI_TMK_DATA_5_	"58AEA26E7008BCE37F97BFCD08BF4A92"
#define _MULTI_TMK_DATA_6_	"89B0250E3B3B9B0285086DF8CB2623F4"
#define _MULTI_TMK_DATA_7_	"C4C7924C58153D94E9F49D51ADAD683D"
#define _MULTI_TMK_DATA_8_	"4FE94CAD0410A4C152460DF2C8FE8F64"
#define _MULTI_TMK_DATA_9_	"DA547FE3152A29F82C19DF622C3BBAFB"
#define _MULTI_TMK_DATA_10_	"6B2F02DAA76DD01C40041694D316E07F"
#define _MULTI_TMK_DATA_11_	"23C73D3D970E5768C83E250B4FC2162F"
#define _MULTI_TMK_DATA_12_	"6E0473B6C746C8F4CB2F0E07326B2ACD"
#define _MULTI_TMK_DATA_13_	"5BFB57A27CC831438A51ABF2CE9D5DA1"
#define _MULTI_TMK_DATA_14_	"7A7F38624AEAC4B923329E97761A0837"
#define _MULTI_TMK_DATA_15_	"49F2CD8010A7EA4FC1526BBCBA4583AE"

/* NCCC共15把 Key*/
#define _KEY_TOTAL_COUNT_	1
//#define _KEY_TOTAL_COUNT_	15

/* Test Key KCV */
#define _MULTI_TMK_KCV_1_	"D5D44FF72068"
#define _MULTI_TMK_KCV_2_	"5DDB28888304"
#define _MULTI_TMK_KCV_3_	"BB48C52B4161"
#define _MULTI_TMK_KCV_4_	"CE155617D835"
#define _MULTI_TMK_KCV_5_	"77EF9820083B"
#define _MULTI_TMK_KCV_6_	"E0C843A4D20E"
#define _MULTI_TMK_KCV_7_	"C0AC2CE7368C"
#define _MULTI_TMK_KCV_8_	"F5B6737268B4"
#define _MULTI_TMK_KCV_9_	"CE532F37192E"
#define _MULTI_TMK_KCV_10_	"4D7F471FE083"
#define _MULTI_TMK_KCV_11_	"4500AB40063B"
#define _MULTI_TMK_KCV_12_	"63D29E3CAE7E"
#define _MULTI_TMK_KCV_13_	"08AD8D5DD798"
#define _MULTI_TMK_KCV_14_	"4ECC1C165483"
#define _MULTI_TMK_KCV_15_	"792658B2E53A"

#define _KCV_TOTAL_COUNT_	15

int inNCCC_TMK_Write_Test_TerminalMasterKey_Flow(void);
int inNCCC_TMK_Write_Test_TerminalMasterKey(void);
int inNCCC_TMK_Write_TerminalMasterKey_By_KeyCard(int inKeyCnt, char *szKeyDataAscii);
int inNCCC_TMK_Write_PINKey(unsigned short usTMKindex, unsigned short usPINLen, char *szPINKey, char* szKeyCheckValueAscii);
int inNCCC_TMK_Write_MACKey(unsigned short usTMKindex, unsigned short usMACKeyLen, char *szMACKeyAscii, char* szKeyCheckValueAscii);
int inNCCC_TMK_Write_ESCKey(TRANSACTION_OBJECT *pobTran);
int inNCCC_TMK_ESC_3DES_Encrypt(char* szInPlaindata, int inInPlaindataLen, char *szResult);
int inNCCC_TMK_CalculatePINBlock(TRANSACTION_OBJECT* pobTran, char *szOutputPINBlock);
int inNCCC_TMK_CalculateMac(char* szInPlaindata, char *szMACdata);
int inNCCC_TMK_GetKeyInfo_LookUp_Default(void);
int inNCCC_TMK_ProductionKey_Swap_To_Temp(void);
int inNCCC_TMK_Check_Test_Key(unsigned short usKeySet, unsigned short usKeyIndex, char *szTest);

int inNCCC_TMK_Test(void);
int inNCCC_TMK_ESCKey_Test(void);
int inNCCC_TMK_MFES_Test(void);

int inFUBON_KEK_Write(BYTE *szKeyData,unsigned short usKeyLength);
int inFUBON_TMK_Write(char usProtectionMode,BYTE *szKeyData,unsigned short usKeyLength,BYTE *szKeyCheckValueAscii,unsigned short usKeyCheckValueLength);
int inNCCC_TMK_GetKeyCheckValue(unsigned short usKeySet, unsigned short usKeyIndex, unsigned char *uszCheckValue);
int inNCCC_GetMkeyInfo(void);
int inNCCC_GetPkeyInfo(void);
