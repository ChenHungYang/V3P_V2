
/*
 * 
 * [檢查參數檔] 此檔案有初始設定值，會由TMS下載檔案，下載後會直接使用TMS下載的檔案  2022/8/12 上午 10:07 [SAM] 
 * 
 */

typedef struct
{
        char    szApplicationIndex[2 + 1];                      /* EMV應用程式索引 */
        char    szMVTApplicationId[16 + 1];                     /* 應用程式 ID  */
        char    szTerminalType[2 + 1];                          /* Terminal_Type 9F35*/
        char    szTerminalCapabilities[6 + 1];                  /* Terminal_Capabilities 9F33*/
        char    szAdditionalTerminalCapabilities[10 + 1];       /* Additional_Terminal_Capabilities */
        char    szTerminalCountryCode[4 + 1];                   /* Terminal_Country_Code */
        char    szTransactionCurrencyCode[4 + 1];               /* Transaction_Currency_Code */
        char    szDefaultTAC[10 + 1];                           /* Default_TAC */
        char    szOnlineTAC[10 + 1];                            /* Online_TAC  */
        char    szDenialTAC[10 + 1];                            /* Denial_TAC */
        char    szDefaultTDOL[6 + 1];                           /* Default_TDOL */
        char    szDefaultDDOL[6 + 1];                           /* Default_DDOL */
        char    szEMVFloorLimit[12 + 1];                        /* EMV_Floor_Limit */
        char    szRandomSelectionThreshold[12 + 1];             /* Random_Selection_Threshol */
        char    szTargetPercentforRandomSelection[2 + 1];       /* Target_Percent_for_Random_Selection */
        char    szMaxTargetPercentforRandomSelection[2 + 1];    /* Max_Target_Percent_for_Random_Selection */
        char    szMerchantCategoryCode[4 + 1];                  /* Merchant Category Code(MCC) */
        char    szTransactionCategoryCode[1 + 1];               /* Transaction Category Code(TCC) */
        char    szTransactionReferenceCurrencyCode[4 + 1];      /* Transaction_Reference_Currency_Code */
        char    szTransactionReferenceCurrencyCoversion[12 + 1];/* Transaction_Reference_Currency_Coversion */
        char    szTransactionReferenceCurrencyExponent[1 + 1];  /* Transaction_Reference_Currency_Exponent */
        char    szEMVPINEntryTimeout[3 + 1];                    /* 等候EMV Offline PIN輸入的Time out時間。 */
} MVT_REC;


/* 以欄位數決定 ex:欄位數是22個，理論上會有21個comma和兩個byte的0x0D 0X0A */
#define _SIZE_MVT_COMMA_0D0A_	23
#define _SIZE_MVT_REC_			(sizeof(MVT_REC))
#define _MVT_FILE_NAME_			"MVT.dat"
#define _MVT_FILE_NAME_BAK_		"MVT.bak"


#define _VISA_MVT_INDEX_				0
#define _MCHIP_MVT_INDEX_			1
#define _JSMART_MVT_INDEX_			2
#define _CUP_DEBIT_MVT_INDEX_		3
#define _CUP_CREDIT_MVT_INDEX_		4
#define _CUP_QUASI_CREDIT_MVT_INDEX_	5
#define _AEIPS_MVT_INDEX_			6
#define _NCCC_MVT_INDEX_			7
#define _D_PAS_MVT_INDEX_			8

/* Load & Save function */
int inLoadMVTRec(int inMVTRec);
int inSaveMVTRec(int inMVTRec);

/* Set function */
int inSetApplicationIndex(char* szApplicationIndex);
int inSetMVTApplicationId(char* szMVTApplicationId);
int inSetMVTTerminalType(char* szTerminalType);
int inSetTerminalCapabilities(char* szTerminalCapabilities);
int inSetAdditionalTerminalCapabilities(char* szAdditionalTerminalCapabilities);
int inSetMVTTerminalCountryCode(char* szTerminalCountryCode);
int inSetMVTTransactionCurrencyCode(char* szTransactionCurrencyCode);
int inSetDefaultTAC(char* szDefaultTAC);
int inSetOnlineTAC(char* szOnlineTAC);
int inSetDenialTAC(char* szDenialTAC);
int inSetDefaultTDOL(char* szDefaultTDOL);
int inSetDefaultDDOL(char* szDefaultDDOL);
int inSetEMVFloorLimit(char* szEMVFloorLimit);
int inSetRandomSelectionThreshold(char* szRandomSelectionThreshold);
int inSetTargetPercentforRandomSelection(char* szTargetPercentforRandomSelection);
int inSetMaxTargetPercentforRandomSelection(char* szMaxTargetPercentforRandomSelection);
int inSetMVTMerchantCategoryCode(char* szMerchantCategoryCode);
int inSetMVTTransactionCategoryCode(char* szTransactionCategoryCode);
int inSetTransactionReferenceCurrencyCode(char* szTransactionReferenceCurrencyCode);
int inSetTransactionReferenceCurrencyCoversion(char* szTransactionReferenceCurrencyCoversion);
int inSetTransactionReferenceCurrencyExponent(char* szTransactionReferenceCurrencyExponent);
int inSetEMVPINEntryTimeout(char* szEMVPINEntryTimeout);



/* Get function */
int inGetApplicationIndex(char* szApplicationIndex);
int inGetMVTApplicationId(char* szMVTApplicationId);
int inGetMVTTerminalType(char* szTerminalType);
int inGetTerminalCapabilities(char* szTerminalCapabilities);
int inGetAdditionalTerminalCapabilities(char* szAdditionalTerminalCapabilities);
int inGetMVTTerminalCountryCode(char* szTerminalCountryCode);
int inGetMVTTransactionCurrencyCode(char* szTransactionCurrencyCode);
int inGetDefaultTAC(char* szDefaultTAC);
int inGetOnlineTAC(char* szOnlineTAC);
int inGetDenialTAC(char* szDenialTAC);
int inGetDefaultTDOL(char* szDefaultTDOL);
int inGetDefaultDDOL(char* szDefaultDDOL);
int inGetEMVFloorLimit(char* szEMVFloorLimit);
int inGetRandomSelectionThreshold(char* szRandomSelectionThreshold);
int inGetTargetPercentforRandomSelection(char* szTargetPercentforRandomSelection);
int inGetMaxTargetPercentforRandomSelection(char* szMaxTargetPercentforRandomSelection);
int inGetMVTMerchantCategoryCode(char* szMerchantCategoryCode);
int inGetMVTTransactionCategoryCode(char* szTransactionCategoryCode);
int inGetTransactionReferenceCurrencyCode(char* szTransactionReferenceCurrencyCode);
int inGetTransactionReferenceCurrencyCoversion(char* szTransactionReferenceCurrencyCoversion);
int inGetTransactionReferenceCurrencyExponent(char* szTransactionReferenceCurrencyExponent);
int inGetEMVPINEntryTimeout(char* szEMVPINEntryTimeout);
