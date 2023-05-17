
typedef struct  EventMenuItem
{
    int     inEventCode;				/* 儲存按鍵 */
    int     inPasswordLevel;			/* 輸入密碼的層級 */
    int     inCode;					/* 交易的類型 */
    int     inRunOperationID;			/* 執行 OPT.txt */
    int     inRunTRTID;				/* 執行 xxxxTRT.txt */
    long    lnHGTransactionType;		/* 聯合_HAPPY GO_交易類別 */
    unsigned char   uszCUPTransBit;	/* 標示是否做銀聯交易 */
    unsigned char   uszInstallmentBit;	/* 標示是否為分期交易 */
    unsigned char   uszRedeemBit;		/* 標示是否為紅利交易 */
    unsigned char   uszECRBit;		/* 標示是否為收銀機交易 */
    unsigned char   uszAutoSettleBit;	/* 標示是否為連動結帳 */
    unsigned char   uszFISCTransBit;	/* 標示是否為SmartPay交易 */
    unsigned char   uszMailOrderBit;	/* 標示是否為郵購交易 */
    unsigned char   uszMultiFuncSlaveBit;	/* 標示是被外接設備 */
    unsigned char   uszESVCTransBit;		/* 標示是否是電票交易 */
} EventMenuItem;

/* 用以確認功能是否有開 */
typedef	struct	MENU_CHECK_TABLE
{
    int     inButtonPositionID;	/* 按鈕位置 */
    int     inCode;			/* 交易別 */
    int     (*inCheckFunc)(int)	/* 用來檢查的function */;
    char    szFileName[100 + 1];	/* 要反白的圖 */
} MENU_CHECK_TABLE;


typedef int (*IN_EVENT_UI_P)(EventMenuItem *srEventMenuItem);



#define _ACCESS_FREELY_                  0
#define _ACCESS_WITH_MANAGER_PASSWORD_   1
#define _ACCESS_WITH_SUPER_PASSWORD_     2
#define _ACCESS_WITH_CLERK_PASSWORD_     3
#define _ACCESS_WITH_CUSTOM_             4
#define _ACCESS_WITH_USER_PASSWORD_      5
#define _ACCESS_WITH_MERCHANT_PASSWORD_  6
#define _ACCESS_WITH_FUNC_PASSWORD_      7
#define _ACCESS_WITH_CUSTOM_RESERVED1_   8
#define _ACCESS_WITH_CUSTOM_RESERVED2_   9

#define _PAGE_1_		1
#define _PAGE_2_		2
#define _PAGE_3_		3
#define _PAGE_4_		4
#define _PAGE_5_		5
#define _PAGE_6_		6
#define _PAGE_7_		7
#define _PAGE_8_		8
#define _PAGE_9_		9
#define _PAGE_10_		10

/*for Touch*/
#define _TOUCH_PAGE_1_		1
#define _TOUCH_PAGE_2_		2
#define _TOUCH_PAGE_3_		3
#define _TOUCH_PAGE_4_		4
#define _TOUCH_PAGE_5_		5
#define _TOUCH_PAGE_6_		6
#define _TOUCH_PAGE_7_		7
#define _TOUCH_PAGE_8_		8
#define _TOUCH_PAGE_9_		9
#define _TOUCH_PAGE_10_		10
#define _TOUCH_PAGE_11_		11
#define _TOUCH_PAGE_12_		12
#define _TOUCH_PAGE_13_		13

#define _MENU_000_NORMAL_ 0

/* 在第幾層迴圈 */
#define _PAGE_LOOP_0_		0	/* 選定就設成0 */
#define _PAGE_LOOP_1_		1
#define _PAGE_LOOP_2_		2
#define _PAGE_LOOP_3_		3
#define _PAGE_LOOP_4_		4

/* Idle UI*/
int inMENU_Decide_Idle_Menu(void);
int inMENU_Old_UI_V3UL(void);
int inMENU_New_UI_MP200(void);
int inMENU_New_UI(void);
int inMENU_Load_Key_UI(void);

/* 第一層 功能UI */
int inMENU_000_MenuFlow(EventMenuItem *srEventMenuItem);
int inMENU_000_MenuFlow_V3UL(EventMenuItem *srEventMenuItem);
int inMENU_000_MenuFlow_MP200(EventMenuItem *srEventMenuItem);
int inMENU_000_MenuFlow_NEWUI(EventMenuItem *srEventMenuItem);
int inMENU_000_MenuFlow_LoadKeyUI(EventMenuItem *srEventMenuItem);
int inMENU_SELECT_KEY_F3(EventMenuItem *srEventMenuItem);
int inMENU_SELECT_KEY_FUNC(EventMenuItem *srEventMenuItem);

int inMENU_Engineer_Fuction(EventMenuItem *srEventMenuItem);


int inMENU_ECR_OPERATION_LOAD_KEY_FROM_520(EventMenuItem *srEventMenuItem);


int inMENU_Check_SETTLE_Enable(int inCode);
int inMENU_Check_Adjust(int inCode);
int inMENU_Check_Transaction_Enable(int inCode);
int inMENU_Check_CUP_Enable(int inCode);
int inMENU_Check_SMARTPAY_Enable(int inCode);
int inMENU_Check_ETICKET_Enable(int inCode);
int inMENU_Check_AWARD_Enable(int inCode);
int inMENU_Check_HG_Enable(int inCode);
int inMENU_Display_ICON(char* szFileName, int inButtonPostionID);
int inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(MENU_CHECK_TABLE *srMENU_CHECK_TABLE);
int inMENU_Check_REVIEW_SETTLE_Enable(int inCode);

/* 子選單 */
int inMENU_FUN3_COMM_TIME_SET(EventMenuItem *srEventMenuItem);
int inMENU_FUN5_TMS(EventMenuItem *srEventMenuItem);
int inMENU_FUN6_SELECT(EventMenuItem *srEventMenuItem);
int inMENU_AWARD_SWIPE(EventMenuItem *srEventMenuItem);
int inMENU_AWARD_BARCODE(EventMenuItem *srEventMenuItem);
int inMENU_AWARD_VOID(EventMenuItem *srEventMenuItem);
int inMENU_DCC_RATE(EventMenuItem *srEventMenuItem);
int inMENU_Download_Parameter(EventMenuItem *srEventMenuItem);
int inMENU_VOID(EventMenuItem *srEventMenuItem);
int inMENU_REFUND(EventMenuItem *srEventMenuItem);
int inMENU_CREDIT_REFUND(EventMenuItem *srEventMenuItem);
int inMENU_REFUND_NORMAL(EventMenuItem *srEventMenuItem);
int inMENU_REFUND_REDEEM(EventMenuItem *srEventMenuItem);
int inMENU_REFUND_INST(EventMenuItem *srEventMenuItem);
int inMENU_INST_REDEEM(EventMenuItem *srEventMenuItem);
int inMENU_REDEEM(EventMenuItem *srEventMenuItem);
int inMENU_INST(EventMenuItem *srEventMenuItem);
int inMENU_SETTLE(EventMenuItem *srEventMenuItem);
int inMENU_SALEOFFLINE(EventMenuItem *srEventMenuItem);
int inMENU_TIP(EventMenuItem *srEventMenuItem);
int inMENU_PREAUTH(EventMenuItem *srEventMenuItem);
int inMENU_MAILORDER(EventMenuItem *srEventMenuItem);
int inMENU_SALE(EventMenuItem *srEventMenuItem);
int inMENU_ADJUST(EventMenuItem *srEventMenuItem);
int inMENU_SALE_ADJUST(EventMenuItem *srEventMenuItem);
int inMENU_REDEEM_ADJUST(EventMenuItem *srEventMenuItem);
int inMENU_INST_ADJUST(EventMenuItem *srEventMenuItem);
int inMENU_FISC_MENU(EventMenuItem *srEventMenuItem);
int inMENU_FISC_SALE(EventMenuItem *srEventMenuItem);
int inMENU_FISC_VOID(EventMenuItem *srEventMenuItem);
int inMENU_FISC_REFUND(EventMenuItem *srEventMenuItem);
int inMENU_SETTLE_AUTOSETTLE(EventMenuItem *srEventMenuItem);
int inMENU_SETTLE_BY_HOST(EventMenuItem *srEventMenuItem);
int inMENU_CUP_SALE(EventMenuItem *srEventMenuItem);
int inMENU_CUP_VOID(EventMenuItem *srEventMenuItem);
int inMENU_CUP_REFUND(EventMenuItem *srEventMenuItem);
int inMENU_CUP_REFUND_NORMAL(EventMenuItem *srEventMenuItem);
int inMENU_CUP_REFUND_MAILORDER(EventMenuItem *srEventMenuItem);
int inMENU_CUP_PREAUTH(EventMenuItem *srEventMenuItem);
int inMENU_CUP_MAILORDER(EventMenuItem *srEventMenuItem);
int inMENU_CUP_INST_REDEEM(EventMenuItem *srEventMenuItem);
int inMENU_CUP_INST(EventMenuItem *srEventMenuItem);
int inMENU_CUP_REDEEM(EventMenuItem *srEventMenuItem);
int inMENU_CUP_LOGON(EventMenuItem *srEventMenuItem);
int inMENU_REPRINT(EventMenuItem *srEventMenuItem);
int inMENU_REVIEW(EventMenuItem *srEventMenuItem);
int inMENU_TOTAL_REVIEW(EventMenuItem *srEventMenuItem);
int inMENU_DETAIL_REVIEW(EventMenuItem *srEventMenuItem);
int inMENU_TMS_PARAMETER_PRINT(EventMenuItem *srEventMenuItem);
int inMENU_DCC_PARAMETER_PRINT(EventMenuItem *srEventMenuItem);
int inMENU_TOTAL_REPORT(EventMenuItem *srEventMenuItem);
int inMENU_DETAIL_REPORT(EventMenuItem *srEventMenuItem);
int inMENU_SAM_REGISTER(EventMenuItem *srEventMenuItem);
int inMENU_EDIT_PASSWORD(EventMenuItem *srEventMenuItem);
int inMENU_TRACELOG_UP(EventMenuItem *srEventMenuItem);
int inMENU_CHECK_VERSION(EventMenuItem *srEventMenuItem);
int inMENU_COMM_SETTING(EventMenuItem *srEventMenuItem);
int inMENU_TIME_SETTING(EventMenuItem *srEventMenuItem);
int inMENU_TMS_PARAMETER_DOWNLOAD(EventMenuItem *srEventMenuItem);
int inMENU_DCC_PARAMETER_DOWNLOAD(EventMenuItem *srEventMenuItem);
int inMENU_TMS_TASK_REPORT(EventMenuItem *srEventMenuItem);
int inMENU_DELETE_BATCH(EventMenuItem *srEventMenuItem);
int inMENU_DEBUG_SWITCH(EventMenuItem *srEventMenuItem);
int inMENU_EDIT_TMEP_VERSION_ID(EventMenuItem *srEventMenuItem);
int inMENU_EDIT_TMSOK(EventMenuItem *srEventMenuItem);
int inMENU_UNLOCK_EDC(EventMenuItem *srEventMenuItem);
int inMENU_REBOOT(EventMenuItem *srEventMenuItem);
int inMENU_DOWNLOAD_CUP_TEST_KEY(EventMenuItem *srEventMenuItem);
int inMENU_EXIT_AP(EventMenuItem *srEventMenuItem);
int inMENU_CHECK_FILE(EventMenuItem *srEventMenuItem);



int inMENU_NEWUI_ETICKET_MENU(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_AWARD_MENU(EventMenuItem *srEventMenuItem);

int inMENU_CHECK_FILE_In_SD(EventMenuItem *srEventMenuItem);
int inMENU_CHECK_FILE_In_USB(EventMenuItem *srEventMenuItem);

/* [新增電票悠遊卡功能] 新增電票使用函式 [SAM] 2022/6/13 下午 2:55 */
int inMENU_ETICKET_DEDUCT(EventMenuItem *srEventMenuItem);
int inMENU_ETICKET_REFUND(EventMenuItem *srEventMenuItem);
int inMENU_ETICKET_INQUIRY(EventMenuItem *srEventMenuItem);
int inMENU_ETICKET_TOP_UP(EventMenuItem *srEventMenuItem);
int inMENU_ETICKET_VOID_TOP_UP(EventMenuItem *srEventMenuItem);
