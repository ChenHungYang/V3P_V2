/*
 * File:   DisTouch.h
 * Author: carolyn
 *
 * Created on 2015年12月3日, 下午 4:37
 */

/* 觸控檔案 */
#define _TOUCH_FILE_    "/dev/input/event0"

typedef enum
{
	_Touch_NONE_ =	200,		/* 不偵測點擊 */
	_Touch_IDLE_,
	_Touch_MENU_,
	_Touch_UNIONPAY_,
	_Touch_OTHER_,
	_Touch_TICKET_,
	_Touch_SIGNATURE_,
	_Touch_FUNCTION_,
	_Touch_FUNCOTHER_,
	_Touch_DCC_CURENCY_OPT_,
	_Touch_DCC_CURENCY_CHECK_,
	_Touch_CUP_LOGON_,
	_Touch_TICKET_CHECK_AMOUNT_,
	_Touch_TICKET_CHECK_RESULT_,
	_Touch_12X19_OPT_,
	_Touch_8X16_OPT_,
	_Touch_16X22_OPT_,
	_Touch_CUST_RECEIPT_,
	_Touch_CHECK_PWD_EDIT_,
	_Touch_HG_SELECT_,
	_Touch_SIGN_CHECK_,
	_Touch_NOSIGN_CHECK_,
	_Touch_NEWUI_IDLE_,
	_Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_,
	_Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_,
	_Touch_NEWUI_FUNC_LINE_2_TO_7_3X4_,
	_Touch_NEWUI_CHOOSE_HOST_,
	_Touch_NEWUI_REVIEW_TOTAL_,
	_Touch_NEWUI_REVIEW_BATCH_,
	_Touch_BATCH_END_,
	_Touch_OX_LINE8_8_,		/* 八行時偵測第八行的確認或取消 */
	_Touch_O_LINE8_8_,		/* 八行時偵測第八行的確認 */
	_Touch_X_LINE8_8_,		/* 八行時偵測第八行的取消 */
	_Touch_TEST_,		
	/* 回傳反應 Start */
	_DisTouch_No_Event_,		/* 沒反應 */
	_DisTouch_Slide_Left_To_Right_,	/* 左到右 */
	_DisTouch_Slide_Right_To_Left_,	/* 右到左 */
	_DisTouch_Slide_Up_To_Down_,	/* 上到下 */
	_DisTouch_Slide_Down_To_Up_,	/* 下到上 */
	/* 回傳反應 End */
		
	/* Idle 按鈕 */
	_IdleTouch_KEY_1_,
	_IdleTouch_KEY_2_,
	_IdleTouch_KEY_3_,

	/* 選擇交易的按鈕*/
	_MenuTouch_KEY_1_,	
	_MenuTouch_KEY_2_,
	_MenuTouch_KEY_3_,
	_MenuTouch_KEY_4_,
	_MenuTouch_KEY_5_,
	_MenuTouch_KEY_6_,
	_MenuTouch_KEY_7_,
	_MenuTouch_KEY_8_,
	_MenuTouch_KEY_9_,
	_MenuTouch_KEY_10_,
	_MenuTouch_KEY_11_,
	_MenuTouch_KEY_12_,
	_MenuTouch_KEY_13_,
	_MenuTouch_KEY_14_,
	_MenuTouch_KEY_15_,
	_MenuTouch_KEY_16_,

	/* 選擇銀聯交易的按鈕*/
	_UnionpayTouch_KEY_1_,
	_UnionpayTouch_KEY_2_,
	_UnionpayTouch_KEY_3_,
	_UnionpayTouch_KEY_4_,
	_UnionpayTouch_KEY_5_,
	_UnionpayTouch_KEY_6_,
	_UnionpayTouch_KEY_7_,
	_UnionpayTouch_KEY_8_,
	_UnionpayTouch_KEY_9_,
		
	/* 選擇其他的按鈕*/
	_OtherTouch_KEY_1_,
	_OtherTouch_KEY_2_,
	_OtherTouch_KEY_3_,
	_OtherTouch_KEY_4_,
	_OtherTouch_KEY_5_,
	_OtherTouch_KEY_6_,
	_OtherTouch_KEY_7_,
	_OtherTouch_KEY_8_,
	_OtherTouch_KEY_9_,
		
	/* 票證的按鈕 */
	_TicketTouch_KEY_1_,
	_TicketTouch_KEY_2_,
	_TicketTouch_KEY_3_,
	_TicketTouch_KEY_4_,
	_TicketTouch_KEY_5_,
	_TicketTouch_KEY_6_,
	_TicketTouch_KEY_7_,
	_TicketTouch_KEY_8_,
	_TicketTouch_KEY_9_,
		
	/* 電子簽名的按鈕 */
	_SignTouch_Clear_,
	_SignTouch_Ok_,
	_SignTouch_Rotate_,
	_SignTouch_Signpad_,

	_SignTouch_Right_Clear_,
	_SignTouch_Right_Ok_,
	_SignTouch_Right_Rotate_,
	_SignTouch_Right_Signpad_,

	/* 功能畫面的按鈕 */
	_FunctionTouch_KEY_1_,
	_FunctionTouch_KEY_2_,
	_FunctionTouch_KEY_3_,
	_FunctionTouch_KEY_4_,
	_FunctionTouch_KEY_5_,
	_FunctionTouch_KEY_6_,
	_FunctionTouch_KEY_7_,
	_FunctionTouch_KEY_8_,
	_FunctionTouch_KEY_9_,
	_FunctionTouch_KEY_10_,
	_FunctionTouch_KEY_11_,
	_FunctionTouch_KEY_12_,
	_FunctionTouch_KEY_13_,
	_FunctionTouch_KEY_14_,
	_FunctionTouch_KEY_15_,

	/* 功能畫面的其他按鈕 */
	_FuncOtherTouch_KEY_1_,
	_FuncOtherTouch_KEY_2_,
	_FuncOtherTouch_KEY_3_,
	_FuncOtherTouch_KEY_4_,
	_FuncOtherTouch_KEY_5_,
	_FuncOtherTouch_KEY_6_,
	_FuncOtherTouch_KEY_7_,
	_FuncOtherTouch_KEY_8_,
	_FuncOtherTouch_KEY_9_,
	_FuncOtherTouch_KEY_10_,
	_FuncOtherTouch_KEY_11_,
	_FuncOtherTouch_KEY_12_,
	_FuncOtherTouch_KEY_13_,
	_FuncOtherTouch_KEY_14_,
	_FuncOtherTouch_KEY_15_,

	/* DCC */
	_DccCurOptionTouch_KEY_1_,
	_DccCurOptionTouch_KEY_2_,
	_DccCurOptionTouch_KEY_3_,
	_DccCurOptionTouch_KEY_4_,
	_DccCurOptionTouch_KEY_5_,
	_DccCurOptionTouch_KEY_6_,
	_DccCurOptionTouch_KEY_7_,
	_DccCurOptionTouch_KEY_8_,
	_DccCurOptionTouch_KEY_9_,
	_DccCurOptionTouch_KEY_10_,
	_DccCurOption_Page_1_,
	_DccCurOption_Page_2_,

	_DccCurCheckTouch_KEY_1_,
	_DccCurCheckTouch_KEY_2_,
		
	/* CUP LogOn */
	_CUPLogOn_Touch_KEY_1_,
	_CUPLogOn_Touch_KEY_2_,
	
	/* _TICKET_AMT_CHECK_*/
	_TICKET_AMT_CHECK_Touch_KEY_1_,
	_TICKET_AMT_CHECK_Touch_KEY_2_,
	
	/* _TICKET_RESULT_CHECK_ */
	_TICKET_RESULT_CHECK_Touch_ENTER_,
	
	/* 12X19 OPT */	
	_OPTTouch12X19_LINE_1_,
	_OPTTouch12X19_LINE_2_,
	_OPTTouch12X19_LINE_3_,
	_OPTTouch12X19_LINE_4_,
	_OPTTouch12X19_LINE_5_,
	_OPTTouch12X19_LINE_6_,
	_OPTTouch12X19_LINE_7_,
	_OPTTouch12X19_LINE_8_,
	_OPTTouch12X19_LINE_9_,
	_OPTTouch12X19_LINE_10_,
	_OPTTouch12X19_LINE_11_,
	_OPTTouch12X19_LINE_12_,
	
	/* 8X16 OPT */	
	_OPTTouch8X16_LINE_1_,
	_OPTTouch8X16_LINE_2_,
	_OPTTouch8X16_LINE_3_,
	_OPTTouch8X16_LINE_4_,
	_OPTTouch8X16_LINE_5_,
	_OPTTouch8X16_LINE_6_,
	_OPTTouch8X16_LINE_7_,
	_OPTTouch8X16_LINE_8_,
		
	/* 16X22 OPT */	
	_OPTTouch16X22_LINE_1_,
	_OPTTouch16X22_LINE_2_,
	_OPTTouch16X22_LINE_3_,
	_OPTTouch16X22_LINE_4_,
	_OPTTouch16X22_LINE_5_,
	_OPTTouch16X22_LINE_6_,
	_OPTTouch16X22_LINE_7_,
	_OPTTouch16X22_LINE_8_,
	_OPTTouch16X22_LINE_9_,
	_OPTTouch16X22_LINE_10_,
	_OPTTouch16X22_LINE_11_,
	_OPTTouch16X22_LINE_12_,
	_OPTTouch16X22_LINE_13_,
	_OPTTouch16X22_LINE_14_,
	_OPTTouch16X22_LINE_15_,
	_OPTTouch16X22_LINE_16_,
		
	/* _Touch_CUST_RECEIPT_ */
	_CUSTReceipt_Touch_ENTER_,
	_CUSTReceipt_Touch_CANCEL_,
		
	/* _Touch_CHECK_EDIT_PWD_ */
	_CHECKEditPWD_Touch_ENTER_,
	
	/* _Touch_HG_SELECT_ */
	_HG_SELECT_Touch_YES_,
	_HG_SELECT_Touch_NO_,
	
	/* _SIGN_CHECK_ */
	_SIGN_CHECK_Touch_KEY_1_,
	_SIGN_CHECK_Touch_KEY_2_,
		
	/* _NOSIGN_CHECK_ */
	_NOSIGN_CHECK_Touch_KEY_1_,	
	_NOSIGN_CHECK_Touch_KEY_2_,
		
	/* NEWUI IDLE */
	_NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_,
		
	/* NEWUI FUNCPAGE_3_TO_8_3X3 */
	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_1_,
	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_2_,
	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_3_,
	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_,
	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_,
	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_,
	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_,
	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_8_,
	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_9_,
		
	/* NEWUI FUNCPAGE_3_TO_7_3X3 */
	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_,
	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_,
	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_,
	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_,
	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_,
	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_,
	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_,
	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_,
	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_,
	
	/* NEWUI FUNCPAGE_2_TO_7_3X4 */
	_NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_1_,
	_NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_2_,
	_NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_3_,
	_NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_4_,
	_NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_5_,
	_NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_6_,
	_NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_7_,
	_NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_8_,
	_NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_9_,
	_NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_10_,
	_NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_11_,
	_NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_12_,

	/* Return Button */
	_NEWUI_LAST_PAGE_BUTTON_,
	_NEWUI_RETURN_IDLE_BUTTON_,	
	
	/* _Touch_NEWUI_CHOOSE_HOST_ */
	_NEWUI_CHOOSE_HOST_Touch_HOST_1_,
	_NEWUI_CHOOSE_HOST_Touch_HOST_2_,
	_NEWUI_CHOOSE_HOST_Touch_HOST_3_,
	_NEWUI_CHOOSE_HOST_Touch_HOST_4_,
	_NEWUI_CHOOSE_HOST_Touch_HOST_5_,
	_NEWUI_CHOOSE_HOST_Touch_HOST_6_,
		
	/* _Touch_NEWUI_REVIEW_TOTAL_ */
	_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_,
	_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_,
		
	/* _Touch_NEWUI_REVIEW_BATCH_ */
	_NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_,
	_NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_,
		
	/* _Touch_BATCH_END_ */
	_BATCH_END_Touch_ENTER_BUTTON_,
		
	/* _Touch_OX_ */
	_Touch_OX_LINE8_8_ENTER_BUTTON_,
	_Touch_OX_LINE8_8_CANCEL_BUTTON_,
		
	/* _Touch_O_ */
	_Touch_O_LINE8_8_ENTER_BUTTON_,
		
	/* _Touch_X_ */
	_Touch_X_LINE8_8_CANCEL_BUTTON_,
			
	_SIGN_FAIL_ENTER_BUTTON_,

	//TODO: 畫面部份可能需要調整, 
	/* [新增電票悠遊卡功能] 財經體系新建畫面 [SAM] 2022/6/9 下午 4:31 */
	_EDC_Touch_ICON_3X4_MENU_,
	_EDC_Touch_ICON_3X3_MENU_,
	_EDC_Touch_ICON_2X2_MENU_,
	_EDC_Touch_ICON_2X4_MENU_,
	_EDC_Touch_ICON_2X5_MENU_,        
	_EDC_Touch_ICON_3X3_FUNC_MENU_,
			
	_EDC_ICON_3X3_KEY_1_,	
	_EDC_ICON_3X3_KEY_2_,
	_EDC_ICON_3X3_KEY_3_,
	_EDC_ICON_3X3_KEY_4_,
	_EDC_ICON_3X3_KEY_5_,
	_EDC_ICON_3X3_KEY_6_,	
	_EDC_ICON_3X3_KEY_7_,
	_EDC_ICON_3X3_KEY_8_,
	_EDC_ICON_3X3_KEY_9_,		
	/* [新增電票悠遊卡功能] 財經體系新建畫面 [SAM] 2022/6/9 下午 4:31 END */
			
	_Touch_CheckConfirmSignature_,/* 新加橫向的簽名確認畫面 20190627 [SAM] */
			
}DISTOUCH;

/* 座標值 START */

/* 標準化按鈕座標值(避免每個都要改) */

/* 4X4 */
#define _DisTouch_4X4_COLUMN1_X1_	((10	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_4X4_COLUMN1_X2_	((75	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_4X4_COLUMN2_X1_	((90	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_4X4_COLUMN2_X2_	((155	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_4X4_COLUMN3_X1_	((168	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_4X4_COLUMN3_X2_	((233	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_4X4_COLUMN4_X1_	((245	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_4X4_COLUMN4_X2_	((310	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)

#define _DisTouch_4X4_LINE1_Y1_		((80	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_4X4_LINE1_Y2_		((155	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_4X4_LINE2_Y1_		((175	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_4X4_LINE2_Y2_		((250	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_4X4_LINE3_Y1_		((273	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_4X4_LINE3_Y2_		((348	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_4X4_LINE4_Y1_		((365	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_4X4_LINE4_Y2_		((445	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* 3X3 */
#define _DisTouch_3X3_COLUMN1_X1_	((20	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_3X3_COLUMN1_X2_	((90	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_3X3_COLUMN2_X1_	((125	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_3X3_COLUMN2_X2_	((195	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_3X3_COLUMN3_X1_	((225	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_3X3_COLUMN3_X2_	((295	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)

#define _DisTouch_3X3_LINE1_Y1_		((195	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_3X3_LINE1_Y2_		((270	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_3X3_LINE2_Y1_		((290	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_3X3_LINE2_Y2_		((365	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_3X3_LINE3_Y1_		((380	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_3X3_LINE3_Y2_		((460	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* 5X3 */
#define _DisTouch_5X3_COLUMN1_X1_	((20	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_5X3_COLUMN1_X2_	((90	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_5X3_COLUMN2_X1_	((125	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_5X3_COLUMN2_X2_	((195	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_5X3_COLUMN3_X1_	((225	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_5X3_COLUMN3_X2_	((295	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)

#define _DisTouch_5X3_LINE1_Y1_		((10	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_5X3_LINE1_Y2_		((85	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_5X3_LINE2_Y1_		((105	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_5X3_LINE2_Y2_		((180	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_5X3_LINE3_Y1_		((200	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_5X3_LINE3_Y2_		((275	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_5X3_LINE4_Y1_		((295	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_5X3_LINE4_Y2_		((370	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_5X3_LINE5_Y1_		((390	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_5X3_LINE5_Y2_		((465	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* NEWUI FUNC 3 To 8 3X3 */
#define _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_	((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X2_	((105	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_	((110	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X2_	((210	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_	((215	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X2_	((315	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)

#define _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y1_	((135	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y2_	((235	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y1_	((245	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y2_	((345	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y1_	((360	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y2_	((460	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* NEWUI FUNC 3 To 7 3X3 */
#define _DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN1_X1_	((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN1_X2_	((105	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN2_X1_	((110	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN2_X2_	((210	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN3_X1_	((215	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN3_X2_	((315	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)

#define _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y1_	((125	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y2_	((205	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y1_	((210	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y2_	((295	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y1_	((300	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y2_	((385	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* NEWUI FUNC 2 To 7 3X4 */
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN1_X1_	((0	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN1_X2_	((100	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN2_X1_	((110	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN2_X2_	((210	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN3_X1_	((220	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN3_X2_	((320	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)

#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE1_Y1_	((60	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE1_Y2_	((135	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE2_Y1_	((155	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE2_Y2_	((230	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE3_Y1_	((250	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE3_Y2_	((325	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE4_Y1_	((345	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE4_Y2_	((420	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* NEWUI CHOOSE HOST 2X3 */
#define _DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN1_X1_	((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN1_X2_	((105	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN2_X1_	((105	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN2_X2_	((205	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN3_X1_	((205	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN3_X2_	((305	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)

#define _DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE1_Y1_	((230	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE1_Y2_	((310	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE2_Y1_	((350	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE2_Y2_	((430	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* 標準化按鈕座標值(END) */

/* IDLE KEY */
#define _DisTouch_IdleTouch_KEY_1_X1_	((20	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_IdleTouch_KEY_1_X2_	((95	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_IdleTouch_KEY_1_Y1_	((355	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_IdleTouch_KEY_1_Y2_	((445	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _DisTouch_IdleTouch_KEY_2_X1_	((120	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_IdleTouch_KEY_2_X2_	((195	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_IdleTouch_KEY_2_Y1_	((355	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_IdleTouch_KEY_2_Y2_	((445	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _DisTouch_IdleTouch_KEY_3_X1_	((220	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_IdleTouch_KEY_3_X2_	((295	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_IdleTouch_KEY_3_Y1_	((355	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _DisTouch_IdleTouch_KEY_3_Y2_	((445	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* MENU KEY */
#define _DisTouch_MenuTouch_KEY_1_X1_	_DisTouch_4X4_COLUMN1_X1_
#define _DisTouch_MenuTouch_KEY_1_X2_	_DisTouch_4X4_COLUMN1_X2_
#define _DisTouch_MenuTouch_KEY_1_Y1_	_DisTouch_4X4_LINE1_Y1_
#define _DisTouch_MenuTouch_KEY_1_Y2_	_DisTouch_4X4_LINE1_Y2_

#define _DisTouch_MenuTouch_KEY_2_X1_	_DisTouch_4X4_COLUMN2_X1_
#define _DisTouch_MenuTouch_KEY_2_X2_	_DisTouch_4X4_COLUMN2_X2_
#define _DisTouch_MenuTouch_KEY_2_Y1_	_DisTouch_4X4_LINE1_Y1_
#define _DisTouch_MenuTouch_KEY_2_Y2_	_DisTouch_4X4_LINE1_Y2_

#define _DisTouch_MenuTouch_KEY_3_X1_	_DisTouch_4X4_COLUMN3_X1_
#define _DisTouch_MenuTouch_KEY_3_X2_	_DisTouch_4X4_COLUMN3_X2_
#define _DisTouch_MenuTouch_KEY_3_Y1_	_DisTouch_4X4_LINE1_Y1_
#define _DisTouch_MenuTouch_KEY_3_Y2_	_DisTouch_4X4_LINE1_Y2_

#define _DisTouch_MenuTouch_KEY_4_X1_	_DisTouch_4X4_COLUMN4_X1_
#define _DisTouch_MenuTouch_KEY_4_X2_	_DisTouch_4X4_COLUMN4_X2_
#define _DisTouch_MenuTouch_KEY_4_Y1_	_DisTouch_4X4_LINE1_Y1_
#define _DisTouch_MenuTouch_KEY_4_Y2_	_DisTouch_4X4_LINE1_Y2_

#define _DisTouch_MenuTouch_KEY_5_X1_	_DisTouch_4X4_COLUMN1_X1_
#define _DisTouch_MenuTouch_KEY_5_X2_	_DisTouch_4X4_COLUMN1_X2_
#define _DisTouch_MenuTouch_KEY_5_Y1_	_DisTouch_4X4_LINE2_Y1_
#define _DisTouch_MenuTouch_KEY_5_Y2_	_DisTouch_4X4_LINE2_Y2_

#define _DisTouch_MenuTouch_KEY_6_X1_	_DisTouch_4X4_COLUMN2_X1_
#define _DisTouch_MenuTouch_KEY_6_X2_	_DisTouch_4X4_COLUMN2_X2_
#define _DisTouch_MenuTouch_KEY_6_Y1_	_DisTouch_4X4_LINE2_Y1_
#define _DisTouch_MenuTouch_KEY_6_Y2_	_DisTouch_4X4_LINE2_Y2_

#define _DisTouch_MenuTouch_KEY_7_X1_	_DisTouch_4X4_COLUMN3_X1_
#define _DisTouch_MenuTouch_KEY_7_X2_	_DisTouch_4X4_COLUMN3_X2_
#define _DisTouch_MenuTouch_KEY_7_Y1_	_DisTouch_4X4_LINE2_Y1_
#define _DisTouch_MenuTouch_KEY_7_Y2_	_DisTouch_4X4_LINE2_Y2_

#define _DisTouch_MenuTouch_KEY_8_X1_	_DisTouch_4X4_COLUMN4_X1_
#define _DisTouch_MenuTouch_KEY_8_X2_	_DisTouch_4X4_COLUMN4_X2_
#define _DisTouch_MenuTouch_KEY_8_Y1_	_DisTouch_4X4_LINE2_Y1_
#define _DisTouch_MenuTouch_KEY_8_Y2_	_DisTouch_4X4_LINE2_Y2_

#define _DisTouch_MenuTouch_KEY_9_X1_	_DisTouch_4X4_COLUMN1_X1_
#define _DisTouch_MenuTouch_KEY_9_X2_	_DisTouch_4X4_COLUMN1_X2_
#define _DisTouch_MenuTouch_KEY_9_Y1_	_DisTouch_4X4_LINE3_Y1_
#define _DisTouch_MenuTouch_KEY_9_Y2_	_DisTouch_4X4_LINE3_Y2_

#define _DisTouch_MenuTouch_KEY_10_X1_	_DisTouch_4X4_COLUMN2_X1_
#define _DisTouch_MenuTouch_KEY_10_X2_	_DisTouch_4X4_COLUMN2_X2_
#define _DisTouch_MenuTouch_KEY_10_Y1_	_DisTouch_4X4_LINE3_Y1_
#define _DisTouch_MenuTouch_KEY_10_Y2_	_DisTouch_4X4_LINE3_Y2_

#define _DisTouch_MenuTouch_KEY_11_X1_	_DisTouch_4X4_COLUMN3_X1_
#define _DisTouch_MenuTouch_KEY_11_X2_	_DisTouch_4X4_COLUMN3_X2_
#define _DisTouch_MenuTouch_KEY_11_Y1_	_DisTouch_4X4_LINE3_Y1_
#define _DisTouch_MenuTouch_KEY_11_Y2_	_DisTouch_4X4_LINE3_Y2_

#define _DisTouch_MenuTouch_KEY_12_X1_	_DisTouch_4X4_COLUMN4_X1_
#define _DisTouch_MenuTouch_KEY_12_X2_	_DisTouch_4X4_COLUMN4_X2_
#define _DisTouch_MenuTouch_KEY_12_Y1_	_DisTouch_4X4_LINE3_Y1_
#define _DisTouch_MenuTouch_KEY_12_Y2_	_DisTouch_4X4_LINE3_Y2_

#define _DisTouch_MenuTouch_KEY_13_X1_	_DisTouch_4X4_COLUMN1_X1_
#define _DisTouch_MenuTouch_KEY_13_X2_	_DisTouch_4X4_COLUMN1_X2_
#define _DisTouch_MenuTouch_KEY_13_Y1_	_DisTouch_4X4_LINE4_Y1_
#define _DisTouch_MenuTouch_KEY_13_Y2_	_DisTouch_4X4_LINE4_Y2_

#define _DisTouch_MenuTouch_KEY_14_X1_	_DisTouch_4X4_COLUMN2_X1_
#define _DisTouch_MenuTouch_KEY_14_X2_	_DisTouch_4X4_COLUMN2_X2_
#define _DisTouch_MenuTouch_KEY_14_Y1_	_DisTouch_4X4_LINE4_Y1_
#define _DisTouch_MenuTouch_KEY_14_Y2_	_DisTouch_4X4_LINE4_Y2_

#define _DisTouch_MenuTouch_KEY_15_X1_	_DisTouch_4X4_COLUMN3_X1_
#define _DisTouch_MenuTouch_KEY_15_X2_	_DisTouch_4X4_COLUMN3_X2_
#define _DisTouch_MenuTouch_KEY_15_Y1_	_DisTouch_4X4_LINE4_Y1_
#define _DisTouch_MenuTouch_KEY_15_Y2_	_DisTouch_4X4_LINE4_Y2_

#define _DisTouch_MenuTouch_KEY_16_X1_	_DisTouch_4X4_COLUMN4_X1_
#define _DisTouch_MenuTouch_KEY_16_X2_	_DisTouch_4X4_COLUMN4_X2_
#define _DisTouch_MenuTouch_KEY_16_Y1_	_DisTouch_4X4_LINE4_Y1_
#define _DisTouch_MenuTouch_KEY_16_Y2_	_DisTouch_4X4_LINE4_Y2_

/* UNIONPAY KEY */
#define _DisTouch_Unionpay_KEY_1_X1_	_DisTouch_3X3_COLUMN1_X1_
#define _DisTouch_Unionpay_KEY_1_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Unionpay_KEY_1_Y1_	_DisTouch_3X3_LINE1_Y1_
#define _DisTouch_Unionpay_KEY_1_Y2_	_DisTouch_3X3_LINE1_Y2_

#define _DisTouch_Unionpay_KEY_2_X1_	_DisTouch_3X3_COLUMN2_X1_
#define _DisTouch_Unionpay_KEY_2_X2_	_DisTouch_3X3_COLUMN2_X2_
#define _DisTouch_Unionpay_KEY_2_Y1_	_DisTouch_3X3_LINE1_Y1_
#define _DisTouch_Unionpay_KEY_2_Y2_	_DisTouch_3X3_LINE1_Y2_

#define _DisTouch_Unionpay_KEY_3_X1_	_DisTouch_3X3_COLUMN3_X1_
#define _DisTouch_Unionpay_KEY_3_X2_	_DisTouch_3X3_COLUMN3_X2_
#define _DisTouch_Unionpay_KEY_3_Y1_	_DisTouch_3X3_LINE1_Y1_
#define _DisTouch_Unionpay_KEY_3_Y2_	_DisTouch_3X3_LINE1_Y2_

#define _DisTouch_Unionpay_KEY_4_X1_	_DisTouch_3X3_COLUMN1_X1_
#define _DisTouch_Unionpay_KEY_4_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Unionpay_KEY_4_Y1_	_DisTouch_3X3_LINE2_Y1_
#define _DisTouch_Unionpay_KEY_4_Y2_	_DisTouch_3X3_LINE2_Y2_

#define _DisTouch_Unionpay_KEY_5_X1_	_DisTouch_3X3_COLUMN2_X1_
#define _DisTouch_Unionpay_KEY_5_X2_	_DisTouch_3X3_COLUMN2_X2_
#define _DisTouch_Unionpay_KEY_5_Y1_	_DisTouch_3X3_LINE2_Y1_
#define _DisTouch_Unionpay_KEY_5_Y2_	_DisTouch_3X3_LINE2_Y2_

#define _DisTouch_Unionpay_KEY_6_X1_	_DisTouch_3X3_COLUMN3_X1_
#define _DisTouch_Unionpay_KEY_6_X2_	_DisTouch_3X3_COLUMN3_X2_
#define _DisTouch_Unionpay_KEY_6_Y1_	_DisTouch_3X3_LINE2_Y1_
#define _DisTouch_Unionpay_KEY_6_Y2_	_DisTouch_3X3_LINE2_Y2_

#define _DisTouch_Unionpay_KEY_7_X1_	_DisTouch_3X3_COLUMN1_X1_
#define _DisTouch_Unionpay_KEY_7_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Unionpay_KEY_7_Y1_	_DisTouch_3X3_LINE3_Y1_
#define _DisTouch_Unionpay_KEY_7_Y2_	_DisTouch_3X3_LINE3_Y2_

#define _DisTouch_Unionpay_KEY_8_X1_	_DisTouch_3X3_COLUMN2_X1_
#define _DisTouch_Unionpay_KEY_8_X2_	_DisTouch_3X3_COLUMN2_X2_
#define _DisTouch_Unionpay_KEY_8_Y1_	_DisTouch_3X3_LINE3_Y1_
#define _DisTouch_Unionpay_KEY_8_Y2_	_DisTouch_3X3_LINE3_Y2_

#define _DisTouch_Unionpay_KEY_9_X1_	_DisTouch_3X3_COLUMN3_X1_
#define _DisTouch_Unionpay_KEY_9_X2_	_DisTouch_3X3_COLUMN3_X2_
#define _DisTouch_Unionpay_KEY_9_Y1_	_DisTouch_3X3_LINE3_Y1_
#define _DisTouch_Unionpay_KEY_9_Y2_	_DisTouch_3X3_LINE3_Y2_

/* OTHER KEY */
#define _DisTouch_Other_KEY_1_X1_	_DisTouch_3X3_COLUMN1_X1_
#define _DisTouch_Other_KEY_1_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Other_KEY_1_Y1_	_DisTouch_3X3_LINE1_Y1_
#define _DisTouch_Other_KEY_1_Y2_	_DisTouch_3X3_LINE1_Y2_

#define _DisTouch_Other_KEY_2_X1_	_DisTouch_3X3_COLUMN2_X1_
#define _DisTouch_Other_KEY_2_X2_	_DisTouch_3X3_COLUMN2_X2_
#define _DisTouch_Other_KEY_2_Y1_	_DisTouch_3X3_LINE1_Y1_
#define _DisTouch_Other_KEY_2_Y2_	_DisTouch_3X3_LINE1_Y2_

#define _DisTouch_Other_KEY_3_X1_	_DisTouch_3X3_COLUMN3_X1_
#define _DisTouch_Other_KEY_3_X2_	_DisTouch_3X3_COLUMN3_X2_
#define _DisTouch_Other_KEY_3_Y1_	_DisTouch_3X3_LINE1_Y1_
#define _DisTouch_Other_KEY_3_Y2_	_DisTouch_3X3_LINE1_Y2_

#define _DisTouch_Other_KEY_4_X1_	_DisTouch_3X3_COLUMN1_X1_	
#define _DisTouch_Other_KEY_4_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Other_KEY_4_Y1_	_DisTouch_3X3_LINE2_Y1_
#define _DisTouch_Other_KEY_4_Y2_	_DisTouch_3X3_LINE2_Y2_

#define _DisTouch_Other_KEY_5_X1_	_DisTouch_3X3_COLUMN2_X1_
#define _DisTouch_Other_KEY_5_X2_	_DisTouch_3X3_COLUMN2_X2_
#define _DisTouch_Other_KEY_5_Y1_	_DisTouch_3X3_LINE2_Y1_
#define _DisTouch_Other_KEY_5_Y2_	_DisTouch_3X3_LINE2_Y2_

#define _DisTouch_Other_KEY_6_X1_	_DisTouch_3X3_COLUMN3_X1_
#define _DisTouch_Other_KEY_6_X2_	_DisTouch_3X3_COLUMN3_X2_
#define _DisTouch_Other_KEY_6_Y1_	_DisTouch_3X3_LINE2_Y1_
#define _DisTouch_Other_KEY_6_Y2_	_DisTouch_3X3_LINE2_Y2_

#define _DisTouch_Other_KEY_7_X1_	_DisTouch_3X3_COLUMN1_X1_
#define _DisTouch_Other_KEY_7_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Other_KEY_7_Y1_	_DisTouch_3X3_LINE3_Y1_
#define _DisTouch_Other_KEY_7_Y2_	_DisTouch_3X3_LINE3_Y2_

#define _DisTouch_Other_KEY_8_X1_	_DisTouch_3X3_COLUMN2_X1_
#define _DisTouch_Other_KEY_8_X2_	_DisTouch_3X3_COLUMN2_X2_
#define _DisTouch_Other_KEY_8_Y1_	_DisTouch_3X3_LINE3_Y1_
#define _DisTouch_Other_KEY_8_Y2_	_DisTouch_3X3_LINE3_Y2_

#define _DisTouch_Other_KEY_9_X1_	_DisTouch_3X3_COLUMN3_X1_
#define _DisTouch_Other_KEY_9_X2_	_DisTouch_3X3_COLUMN3_X2_
#define _DisTouch_Other_KEY_9_Y1_	_DisTouch_3X3_LINE3_Y1_
#define _DisTouch_Other_KEY_9_Y2_	_DisTouch_3X3_LINE3_Y2_

/* FUNCTION KEY */
#define _DisTouch_Function_KEY_1_X1_	_DisTouch_3X3_COLUMN1_X1_
#define _DisTouch_Function_KEY_1_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Function_KEY_1_Y1_	_DisTouch_5X3_LINE1_Y1_
#define _DisTouch_Function_KEY_1_Y2_	_DisTouch_5X3_LINE1_Y2_

#define _DisTouch_Function_KEY_2_X1_	_DisTouch_5X3_COLUMN2_X1_
#define _DisTouch_Function_KEY_2_X2_	_DisTouch_5X3_COLUMN2_X2_
#define _DisTouch_Function_KEY_2_Y1_	_DisTouch_5X3_LINE1_Y1_
#define _DisTouch_Function_KEY_2_Y2_	_DisTouch_5X3_LINE1_Y2_

#define _DisTouch_Function_KEY_3_X1_	_DisTouch_5X3_COLUMN3_X1_
#define _DisTouch_Function_KEY_3_X2_	_DisTouch_5X3_COLUMN3_X2_
#define _DisTouch_Function_KEY_3_Y1_	_DisTouch_5X3_LINE1_Y1_
#define _DisTouch_Function_KEY_3_Y2_	_DisTouch_5X3_LINE1_Y2_

#define _DisTouch_Function_KEY_4_X1_	_DisTouch_3X3_COLUMN1_X1_
#define _DisTouch_Function_KEY_4_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Function_KEY_4_Y1_	_DisTouch_5X3_LINE2_Y1_
#define _DisTouch_Function_KEY_4_Y2_	_DisTouch_5X3_LINE2_Y2_

#define _DisTouch_Function_KEY_5_X1_	_DisTouch_5X3_COLUMN2_X1_
#define _DisTouch_Function_KEY_5_X2_	_DisTouch_5X3_COLUMN2_X2_
#define _DisTouch_Function_KEY_5_Y1_	_DisTouch_5X3_LINE2_Y1_
#define _DisTouch_Function_KEY_5_Y2_	_DisTouch_5X3_LINE2_Y2_

#define _DisTouch_Function_KEY_6_X1_	_DisTouch_5X3_COLUMN3_X1_
#define _DisTouch_Function_KEY_6_X2_	_DisTouch_5X3_COLUMN3_X2_
#define _DisTouch_Function_KEY_6_Y1_	_DisTouch_5X3_LINE2_Y1_
#define _DisTouch_Function_KEY_6_Y2_	_DisTouch_5X3_LINE2_Y2_

#define _DisTouch_Function_KEY_7_X1_	_DisTouch_3X3_COLUMN1_X1_
#define _DisTouch_Function_KEY_7_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Function_KEY_7_Y1_	_DisTouch_5X3_LINE3_Y1_
#define _DisTouch_Function_KEY_7_Y2_	_DisTouch_5X3_LINE3_Y2_

#define _DisTouch_Function_KEY_8_X1_	_DisTouch_5X3_COLUMN2_X1_
#define _DisTouch_Function_KEY_8_X2_	_DisTouch_5X3_COLUMN2_X2_
#define _DisTouch_Function_KEY_8_Y1_	_DisTouch_5X3_LINE3_Y1_
#define _DisTouch_Function_KEY_8_Y2_	_DisTouch_5X3_LINE3_Y2_

#define _DisTouch_Function_KEY_9_X1_	_DisTouch_5X3_COLUMN3_X1_
#define _DisTouch_Function_KEY_9_X2_	_DisTouch_5X3_COLUMN3_X2_
#define _DisTouch_Function_KEY_9_Y1_	_DisTouch_5X3_LINE3_Y1_
#define _DisTouch_Function_KEY_9_Y2_	_DisTouch_5X3_LINE3_Y2_

#define _DisTouch_Function_KEY_10_X1_	_DisTouch_3X3_COLUMN1_X1_
#define _DisTouch_Function_KEY_10_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Function_KEY_10_Y1_	_DisTouch_5X3_LINE4_Y1_
#define _DisTouch_Function_KEY_10_Y2_	_DisTouch_5X3_LINE4_Y2_

#define _DisTouch_Function_KEY_11_X1_	_DisTouch_5X3_COLUMN2_X1_
#define _DisTouch_Function_KEY_11_X2_	_DisTouch_5X3_COLUMN2_X2_
#define _DisTouch_Function_KEY_11_Y1_	_DisTouch_5X3_LINE4_Y1_
#define _DisTouch_Function_KEY_11_Y2_	_DisTouch_5X3_LINE4_Y2_

#define _DisTouch_Function_KEY_12_X1_	_DisTouch_5X3_COLUMN3_X1_
#define _DisTouch_Function_KEY_12_X2_	_DisTouch_5X3_COLUMN3_X2_
#define _DisTouch_Function_KEY_12_Y1_	_DisTouch_5X3_LINE4_Y1_
#define _DisTouch_Function_KEY_12_Y2_	_DisTouch_5X3_LINE4_Y2_

#define _DisTouch_Function_KEY_13_X1_	_DisTouch_3X3_COLUMN1_X1_
#define _DisTouch_Function_KEY_13_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Function_KEY_13_Y1_	_DisTouch_5X3_LINE5_Y1_
#define _DisTouch_Function_KEY_13_Y2_	_DisTouch_5X3_LINE5_Y2_

#define _DisTouch_Function_KEY_14_X1_	_DisTouch_5X3_COLUMN2_X1_
#define _DisTouch_Function_KEY_14_X2_	_DisTouch_5X3_COLUMN2_X2_
#define _DisTouch_Function_KEY_14_Y1_	_DisTouch_5X3_LINE5_Y1_
#define _DisTouch_Function_KEY_14_Y2_	_DisTouch_5X3_LINE5_Y2_

#define _DisTouch_Function_KEY_15_X1_	_DisTouch_5X3_COLUMN3_X1_
#define _DisTouch_Function_KEY_15_X2_	_DisTouch_5X3_COLUMN3_X2_
#define _DisTouch_Function_KEY_15_Y1_	_DisTouch_5X3_LINE5_Y1_
#define _DisTouch_Function_KEY_15_Y2_	_DisTouch_5X3_LINE5_Y2_

/* FUNCTION OTHER KEY */
#define _DisTouch_FuncOther_KEY_1_X1_	_DisTouch_5X3_COLUMN1_X1_
#define _DisTouch_FuncOther_KEY_1_X2_	_DisTouch_5X3_COLUMN1_X2_
#define _DisTouch_FuncOther_KEY_1_Y1_	_DisTouch_5X3_LINE1_Y1_
#define _DisTouch_FuncOther_KEY_1_Y2_	_DisTouch_5X3_LINE1_Y2_

#define _DisTouch_FuncOther_KEY_2_X1_	_DisTouch_5X3_COLUMN2_X1_
#define _DisTouch_FuncOther_KEY_2_X2_	_DisTouch_5X3_COLUMN2_X2_
#define _DisTouch_FuncOther_KEY_2_Y1_	_DisTouch_5X3_LINE1_Y1_
#define _DisTouch_FuncOther_KEY_2_Y2_	_DisTouch_5X3_LINE1_Y2_

#define _DisTouch_FuncOther_KEY_3_X1_	_DisTouch_5X3_COLUMN3_X1_
#define _DisTouch_FuncOther_KEY_3_X2_	_DisTouch_5X3_COLUMN3_X2_
#define _DisTouch_FuncOther_KEY_3_Y1_	_DisTouch_5X3_LINE1_Y1_
#define _DisTouch_FuncOther_KEY_3_Y2_	_DisTouch_5X3_LINE1_Y2_

#define _DisTouch_FuncOther_KEY_4_X1_	_DisTouch_5X3_COLUMN1_X1_
#define _DisTouch_FuncOther_KEY_4_X2_	_DisTouch_5X3_COLUMN1_X2_
#define _DisTouch_FuncOther_KEY_4_Y1_	_DisTouch_5X3_LINE2_Y1_
#define _DisTouch_FuncOther_KEY_4_Y2_	_DisTouch_5X3_LINE2_Y2_

#define _DisTouch_FuncOther_KEY_5_X1_	_DisTouch_5X3_COLUMN2_X1_
#define _DisTouch_FuncOther_KEY_5_X2_	_DisTouch_5X3_COLUMN2_X2_
#define _DisTouch_FuncOther_KEY_5_Y1_	_DisTouch_5X3_LINE2_Y1_
#define _DisTouch_FuncOther_KEY_5_Y2_	_DisTouch_5X3_LINE2_Y2_

#define _DisTouch_FuncOther_KEY_6_X1_	_DisTouch_5X3_COLUMN3_X1_
#define _DisTouch_FuncOther_KEY_6_X2_	_DisTouch_5X3_COLUMN3_X2_
#define _DisTouch_FuncOther_KEY_6_Y1_	_DisTouch_5X3_LINE2_Y1_
#define _DisTouch_FuncOther_KEY_6_Y2_	_DisTouch_5X3_LINE2_Y2_

#define _DisTouch_FuncOther_KEY_7_X1_	_DisTouch_5X3_COLUMN1_X1_
#define _DisTouch_FuncOther_KEY_7_X2_	_DisTouch_5X3_COLUMN1_X2_
#define _DisTouch_FuncOther_KEY_7_Y1_	_DisTouch_5X3_LINE3_Y1_
#define _DisTouch_FuncOther_KEY_7_Y2_	_DisTouch_5X3_LINE3_Y2_

#define _DisTouch_FuncOther_KEY_8_X1_	_DisTouch_5X3_COLUMN2_X1_
#define _DisTouch_FuncOther_KEY_8_X2_	_DisTouch_5X3_COLUMN2_X2_
#define _DisTouch_FuncOther_KEY_8_Y1_	_DisTouch_5X3_LINE3_Y1_
#define _DisTouch_FuncOther_KEY_8_Y2_	_DisTouch_5X3_LINE3_Y2_

#define _DisTouch_FuncOther_KEY_9_X1_	_DisTouch_5X3_COLUMN3_X1_
#define _DisTouch_FuncOther_KEY_9_X2_	_DisTouch_5X3_COLUMN3_X2_
#define _DisTouch_FuncOther_KEY_9_Y1_	_DisTouch_5X3_LINE3_Y1_
#define _DisTouch_FuncOther_KEY_9_Y2_	_DisTouch_5X3_LINE3_Y2_

#define _DisTouch_FuncOther_KEY_10_X1_	_DisTouch_5X3_COLUMN1_X1_
#define _DisTouch_FuncOther_KEY_10_X2_	_DisTouch_5X3_COLUMN1_X2_
#define _DisTouch_FuncOther_KEY_10_Y1_	_DisTouch_5X3_LINE4_Y1_
#define _DisTouch_FuncOther_KEY_10_Y2_	_DisTouch_5X3_LINE4_Y2_

#define _DisTouch_FuncOther_KEY_11_X1_	_DisTouch_5X3_COLUMN2_X1_
#define _DisTouch_FuncOther_KEY_11_X2_	_DisTouch_5X3_COLUMN2_X2_
#define _DisTouch_FuncOther_KEY_11_Y1_	_DisTouch_5X3_LINE4_Y1_
#define _DisTouch_FuncOther_KEY_11_Y2_	_DisTouch_5X3_LINE4_Y2_

#define _DisTouch_FuncOther_KEY_12_X1_	_DisTouch_5X3_COLUMN3_X1_
#define _DisTouch_FuncOther_KEY_12_X2_	_DisTouch_5X3_COLUMN3_X2_
#define _DisTouch_FuncOther_KEY_12_Y1_	_DisTouch_5X3_LINE4_Y1_
#define _DisTouch_FuncOther_KEY_12_Y2_	_DisTouch_5X3_LINE4_Y2_

#define _DisTouch_FuncOther_KEY_13_X1_	_DisTouch_5X3_COLUMN1_X1_
#define _DisTouch_FuncOther_KEY_13_X2_	_DisTouch_5X3_COLUMN1_X2_
#define _DisTouch_FuncOther_KEY_13_Y1_	_DisTouch_5X3_LINE5_Y1_
#define _DisTouch_FuncOther_KEY_13_Y2_	_DisTouch_5X3_LINE5_Y2_

#define _DisTouch_FuncOther_KEY_14_X1_	_DisTouch_5X3_COLUMN2_X1_
#define _DisTouch_FuncOther_KEY_14_X2_	_DisTouch_5X3_COLUMN2_X2_
#define _DisTouch_FuncOther_KEY_14_Y1_	_DisTouch_5X3_LINE5_Y1_
#define _DisTouch_FuncOther_KEY_14_Y2_	_DisTouch_5X3_LINE5_Y2_

#define _DisTouch_FuncOther_KEY_15_X1_	_DisTouch_5X3_COLUMN3_X1_
#define _DisTouch_FuncOther_KEY_15_X2_	_DisTouch_5X3_COLUMN3_X2_
#define _DisTouch_FuncOther_KEY_15_Y1_	_DisTouch_5X3_LINE5_Y1_
#define _DisTouch_FuncOther_KEY_15_Y2_	_DisTouch_5X3_LINE5_Y2_

/* OTHER KEY */
#define _DisTouch_Ticket_KEY_1_X1_	_DisTouch_3X3_COLUMN1_X1_
#define _DisTouch_Ticket_KEY_1_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Ticket_KEY_1_Y1_	_DisTouch_3X3_LINE1_Y1_
#define _DisTouch_Ticket_KEY_1_Y2_	_DisTouch_3X3_LINE1_Y2_

#define _DisTouch_Ticket_KEY_2_X1_	_DisTouch_3X3_COLUMN2_X1_
#define _DisTouch_Ticket_KEY_2_X2_	_DisTouch_3X3_COLUMN2_X2_
#define _DisTouch_Ticket_KEY_2_Y1_	_DisTouch_3X3_LINE1_Y1_
#define _DisTouch_Ticket_KEY_2_Y2_	_DisTouch_3X3_LINE1_Y2_

#define _DisTouch_Ticket_KEY_3_X1_	_DisTouch_3X3_COLUMN3_X1_
#define _DisTouch_Ticket_KEY_3_X2_	_DisTouch_3X3_COLUMN3_X2_
#define _DisTouch_Ticket_KEY_3_Y1_	_DisTouch_3X3_LINE1_Y1_
#define _DisTouch_Ticket_KEY_3_Y2_	_DisTouch_3X3_LINE1_Y2_

#define _DisTouch_Ticket_KEY_4_X1_	_DisTouch_3X3_COLUMN1_X1_	
#define _DisTouch_Ticket_KEY_4_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Ticket_KEY_4_Y1_	_DisTouch_3X3_LINE2_Y1_
#define _DisTouch_Ticket_KEY_4_Y2_	_DisTouch_3X3_LINE2_Y2_

#define _DisTouch_Ticket_KEY_5_X1_	_DisTouch_3X3_COLUMN2_X1_
#define _DisTouch_Ticket_KEY_5_X2_	_DisTouch_3X3_COLUMN2_X2_
#define _DisTouch_Ticket_KEY_5_Y1_	_DisTouch_3X3_LINE2_Y1_
#define _DisTouch_Ticket_KEY_5_Y2_	_DisTouch_3X3_LINE2_Y2_

#define _DisTouch_Ticket_KEY_6_X1_	_DisTouch_3X3_COLUMN3_X1_
#define _DisTouch_Ticket_KEY_6_X2_	_DisTouch_3X3_COLUMN3_X2_
#define _DisTouch_Ticket_KEY_6_Y1_	_DisTouch_3X3_LINE2_Y1_
#define _DisTouch_Ticket_KEY_6_Y2_	_DisTouch_3X3_LINE2_Y2_

#define _DisTouch_Ticket_KEY_7_X1_	_DisTouch_3X3_COLUMN1_X1_
#define _DisTouch_Ticket_KEY_7_X2_	_DisTouch_3X3_COLUMN1_X2_
#define _DisTouch_Ticket_KEY_7_Y1_	_DisTouch_3X3_LINE3_Y1_
#define _DisTouch_Ticket_KEY_7_Y2_	_DisTouch_3X3_LINE3_Y2_

#define _DisTouch_Ticket_KEY_8_X1_	_DisTouch_3X3_COLUMN2_X1_
#define _DisTouch_Ticket_KEY_8_X2_	_DisTouch_3X3_COLUMN2_X2_
#define _DisTouch_Ticket_KEY_8_Y1_	_DisTouch_3X3_LINE3_Y1_
#define _DisTouch_Ticket_KEY_8_Y2_	_DisTouch_3X3_LINE3_Y2_

#define _DisTouch_Ticket_KEY_9_X1_	_DisTouch_3X3_COLUMN3_X1_
#define _DisTouch_Ticket_KEY_9_X2_	_DisTouch_3X3_COLUMN3_X2_
#define _DisTouch_Ticket_KEY_9_Y1_	_DisTouch_3X3_LINE3_Y1_
#define _DisTouch_Ticket_KEY_9_Y2_	_DisTouch_3X3_LINE3_Y2_

/* Signature */
#define _Distouch_Sign_Clear_X1_		((255	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Sign_Clear_X2_		((315	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Sign_Clear_Y1_		((370	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_Sign_Clear_Y2_		((470	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_Sign_Ok_X1_			((255	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Sign_Ok_X2_			((320	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Sign_Ok_Y1_			((260	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_Sign_Ok_Y2_			((360	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_Sign_Rotate_X1_		((250	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Sign_Rotate_X2_		((310	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Sign_Rotate_Y1_		((10	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_Sign_Rotate_Y2_		((70	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)


#define _Distouch_Sign_Right_Clear_X1_		((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Sign_Right_Clear_X2_		((65	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Sign_Right_Clear_Y1_		((10	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_Sign_Right_Clear_Y2_		((110	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_Sign_Right_Ok_X1_		((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Sign_Right_Ok_X2_		((65	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Sign_Right_Ok_Y1_		((120	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_Sign_Right_Ok_Y2_		((220	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_Sign_Right_Rotate_X1_		((10	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Sign_Right_Rotate_X2_		((70	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Sign_Right_Rotate_Y1_		((410	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_Sign_Right_Rotate_Y2_		((470	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* 横向簽名確認用 20170701 [SAM] */
#define _Distouch_Left_Confirm_Sign_Clear_X1_		((254	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Left_Confirm_Sign_Clear_X2_		((315	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Left_Confirm_Sign_Clear_Y1_		((390	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_Left_Confirm_Sign_Clear_Y2_		((470	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_Left_Confirm_Sign_Ok_X1_		((254	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Left_Confirm_Sign_Ok_X2_		((315	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Left_Confirm_Sign_Ok_Y1_		((185	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_Left_Confirm_Sign_Ok_Y2_		((265	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_Right_Confirm_Sign_Clear_X1_		((4	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Right_Confirm_Sign_Clear_X2_		((65	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Right_Confirm_Sign_Clear_Y1_		((15	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_Right_Confirm_Sign_Clear_Y2_		((90	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_Right_Confirm_Sign_Ok_X1_			((4	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Right_Confirm_Sign_Ok_X2_			((65	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_Right_Confirm_Sign_Ok_Y1_			((214	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_Right_Confirm_Sign_Ok_Y2_			((295	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)



/* DCC CURRENCY CHECK */
#define _Distouch_DccCurCheck_KEY_1_X1_		((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurCheck_KEY_1_X2_		((315	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurCheck_KEY_1_Y1_		((55	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_DccCurCheck_KEY_1_Y2_		((95	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_DccCurCheck_KEY_2_X1_		((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurCheck_KEY_2_X2_		((315	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurCheck_KEY_2_Y1_		((115	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_DccCurCheck_KEY_2_Y2_		((155	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* DCC CURRNECY OPTION */
#define _Distouch_DccCurOption_KEY_1_X1_	((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_1_X2_	((150	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_1_Y1_	((80	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_DccCurOption_KEY_1_Y2_	((125	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_DccCurOption_KEY_2_X1_	((170	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_2_X2_	((315	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_2_Y1_	((80	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_DccCurOption_KEY_2_Y2_	((125	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_DccCurOption_KEY_3_X1_	((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_3_X2_	((150	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_3_Y1_	((160	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_DccCurOption_KEY_3_Y2_	((205	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_DccCurOption_KEY_4_X1_	((170	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_4_X2_	((315	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_4_Y1_	((160	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_DccCurOption_KEY_4_Y2_	((205	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_DccCurOption_KEY_5_X1_	((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_5_X2_	((150	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_5_Y1_	((240	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_DccCurOption_KEY_5_Y2_	((285	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_DccCurOption_KEY_6_X1_	((170	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_6_X2_	((315	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_6_Y1_	((240	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_DccCurOption_KEY_6_Y2_	((285	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_DccCurOption_KEY_7_X1_	((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_7_X2_	((150	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_7_Y1_	((320	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_DccCurOption_KEY_7_Y2_	((365	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_DccCurOption_KEY_8_X1_	((170	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_8_X2_	((315	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_8_Y1_	((320	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_DccCurOption_KEY_8_Y2_	((365	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_DccCurOption_KEY_9_X1_	((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_9_X2_	((150	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_9_Y1_	((400	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_DccCurOption_KEY_9_Y2_	((445	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_DccCurOption_KEY_10_X1_	((170	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_10_X2_	((315	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_DccCurOption_KEY_10_Y1_	((400	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_DccCurOption_KEY_10_Y2_	((445	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* CUP LOGON CHECK */
#define _Distouch_CUPLogOn_KEY_1_X1_		((240	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_CUPLogOn_KEY_1_X2_		((310	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_CUPLogOn_KEY_1_Y1_		((375	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_CUPLogOn_KEY_1_Y2_		((405	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_CUPLogOn_KEY_2_X1_		((240	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_CUPLogOn_KEY_2_X2_		((310	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_CUPLogOn_KEY_2_Y1_		((435	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_CUPLogOn_KEY_2_Y2_		((465	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* TICKET AMOUNT CHECK */
#define _Distouch_TICKET_AMT_CHECK_KEY_1_X1_	((105	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TICKET_AMT_CHECK_KEY_1_X2_	((170	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TICKET_AMT_CHECK_KEY_1_Y1_	((370	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_TICKET_AMT_CHECK_KEY_1_Y2_	((405	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_TICKET_AMT_CHECK_KEY_2_X1_	((240	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TICKET_AMT_CHECK_KEY_2_X2_	((310	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TICKET_AMT_CHECK_KEY_2_Y1_	((435	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_TICKET_AMT_CHECK_KEY_2_Y2_	((465	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* TICKET RESULT CHECK */
#define _Distouch_TICKET_RESULT_CHECK_ENTER_X1_	((85	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TICKET_RESULT_CHECK_ENTER_X2_	((165	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TICKET_RESULT_CHECK_ENTER_Y1_	((365	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_TICKET_RESULT_CHECK_ENTER_Y2_	((410	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* 12X19 OPT */
#define _Distouch_OPT12X19_LINE_1_X1_		0
#define _Distouch_OPT12X19_LINE_1_X2_		_LCD_XSIZE_
#define _Distouch_OPT12X19_LINE_1_Y1_		(_LCD_YSIZE_ / 12) * 0
#define _Distouch_OPT12X19_LINE_1_Y2_		(_LCD_YSIZE_ / 12) * 1

#define _Distouch_OPT12X19_LINE_2_X1_		0
#define _Distouch_OPT12X19_LINE_2_X2_		_LCD_XSIZE_
#define _Distouch_OPT12X19_LINE_2_Y1_		(_LCD_YSIZE_ / 12) * 1
#define _Distouch_OPT12X19_LINE_2_Y2_		(_LCD_YSIZE_ / 12) * 2

#define _Distouch_OPT12X19_LINE_3_X1_		0
#define _Distouch_OPT12X19_LINE_3_X2_		_LCD_XSIZE_
#define _Distouch_OPT12X19_LINE_3_Y1_		(_LCD_YSIZE_ / 12) * 2
#define _Distouch_OPT12X19_LINE_3_Y2_		(_LCD_YSIZE_ / 12) * 3

#define _Distouch_OPT12X19_LINE_4_X1_		0
#define _Distouch_OPT12X19_LINE_4_X2_		_LCD_XSIZE_
#define _Distouch_OPT12X19_LINE_4_Y1_		(_LCD_YSIZE_ / 12) * 3
#define _Distouch_OPT12X19_LINE_4_Y2_		(_LCD_YSIZE_ / 12) * 4

#define _Distouch_OPT12X19_LINE_5_X1_		0
#define _Distouch_OPT12X19_LINE_5_X2_		_LCD_XSIZE_
#define _Distouch_OPT12X19_LINE_5_Y1_		(_LCD_YSIZE_ / 12) * 4
#define _Distouch_OPT12X19_LINE_5_Y2_		(_LCD_YSIZE_ / 12) * 5

#define _Distouch_OPT12X19_LINE_6_X1_		0
#define _Distouch_OPT12X19_LINE_6_X2_		_LCD_XSIZE_
#define _Distouch_OPT12X19_LINE_6_Y1_		(_LCD_YSIZE_ / 12) * 5
#define _Distouch_OPT12X19_LINE_6_Y2_		(_LCD_YSIZE_ / 12) * 6

#define _Distouch_OPT12X19_LINE_7_X1_		0
#define _Distouch_OPT12X19_LINE_7_X2_		_LCD_XSIZE_
#define _Distouch_OPT12X19_LINE_7_Y1_		(_LCD_YSIZE_ / 12) * 6
#define _Distouch_OPT12X19_LINE_7_Y2_		(_LCD_YSIZE_ / 12) * 7

#define _Distouch_OPT12X19_LINE_8_X1_		0
#define _Distouch_OPT12X19_LINE_8_X2_		_LCD_XSIZE_
#define _Distouch_OPT12X19_LINE_8_Y1_		(_LCD_YSIZE_ / 12) * 7
#define _Distouch_OPT12X19_LINE_8_Y2_		(_LCD_YSIZE_ / 12) * 8

#define _Distouch_OPT12X19_LINE_9_X1_		0
#define _Distouch_OPT12X19_LINE_9_X2_		_LCD_XSIZE_
#define _Distouch_OPT12X19_LINE_9_Y1_		(_LCD_YSIZE_ / 12) * 8
#define _Distouch_OPT12X19_LINE_9_Y2_		(_LCD_YSIZE_ / 12) * 9

#define _Distouch_OPT12X19_LINE_10_X1_		0
#define _Distouch_OPT12X19_LINE_10_X2_		_LCD_XSIZE_
#define _Distouch_OPT12X19_LINE_10_Y1_		(_LCD_YSIZE_ / 12) * 9
#define _Distouch_OPT12X19_LINE_10_Y2_		(_LCD_YSIZE_ / 12) * 10

#define _Distouch_OPT12X19_LINE_11_X1_		0
#define _Distouch_OPT12X19_LINE_11_X2_		_LCD_XSIZE_
#define _Distouch_OPT12X19_LINE_11_Y1_		(_LCD_YSIZE_ / 12) * 10
#define _Distouch_OPT12X19_LINE_11_Y2_		(_LCD_YSIZE_ / 12) * 11

#define _Distouch_OPT12X19_LINE_12_X1_		0
#define _Distouch_OPT12X19_LINE_12_X2_		_LCD_XSIZE_
#define _Distouch_OPT12X19_LINE_12_Y1_		(_LCD_YSIZE_ / 12) * 11
#define _Distouch_OPT12X19_LINE_12_Y2_		(_LCD_YSIZE_ / 12) * 12

/* 8X16 OPT */
#define _Distouch_OPT8X16_LINE_1_X1_		0
#define _Distouch_OPT8X16_LINE_1_X2_		_LCD_XSIZE_
#define _Distouch_OPT8X16_LINE_1_Y1_		(_LCD_YSIZE_ / 8) * 0
#define _Distouch_OPT8X16_LINE_1_Y2_		(_LCD_YSIZE_ / 8) * 1

#define _Distouch_OPT8X16_LINE_2_X1_		0
#define _Distouch_OPT8X16_LINE_2_X2_		_LCD_XSIZE_
#define _Distouch_OPT8X16_LINE_2_Y1_		(_LCD_YSIZE_ / 8) * 1
#define _Distouch_OPT8X16_LINE_2_Y2_		(_LCD_YSIZE_ / 8) * 2

#define _Distouch_OPT8X16_LINE_3_X1_		0
#define _Distouch_OPT8X16_LINE_3_X2_		_LCD_XSIZE_
#define _Distouch_OPT8X16_LINE_3_Y1_		(_LCD_YSIZE_ / 8) * 2
#define _Distouch_OPT8X16_LINE_3_Y2_		(_LCD_YSIZE_ / 8) * 3

#define _Distouch_OPT8X16_LINE_4_X1_		0
#define _Distouch_OPT8X16_LINE_4_X2_		_LCD_XSIZE_
#define _Distouch_OPT8X16_LINE_4_Y1_		(_LCD_YSIZE_ / 8) * 3
#define _Distouch_OPT8X16_LINE_4_Y2_		(_LCD_YSIZE_ / 8) * 4

#define _Distouch_OPT8X16_LINE_5_X1_		0
#define _Distouch_OPT8X16_LINE_5_X2_		_LCD_XSIZE_
#define _Distouch_OPT8X16_LINE_5_Y1_		(_LCD_YSIZE_ / 8) * 4
#define _Distouch_OPT8X16_LINE_5_Y2_		(_LCD_YSIZE_ / 8) * 5

#define _Distouch_OPT8X16_LINE_6_X1_		0
#define _Distouch_OPT8X16_LINE_6_X2_		_LCD_XSIZE_
#define _Distouch_OPT8X16_LINE_6_Y1_		(_LCD_YSIZE_ / 8) * 5
#define _Distouch_OPT8X16_LINE_6_Y2_		(_LCD_YSIZE_ / 8) * 6

#define _Distouch_OPT8X16_LINE_7_X1_		0
#define _Distouch_OPT8X16_LINE_7_X2_		_LCD_XSIZE_
#define _Distouch_OPT8X16_LINE_7_Y1_		(_LCD_YSIZE_ / 8) * 6
#define _Distouch_OPT8X16_LINE_7_Y2_		(_LCD_YSIZE_ / 8) * 7

#define _Distouch_OPT8X16_LINE_8_X1_		0
#define _Distouch_OPT8X16_LINE_8_X2_		_LCD_XSIZE_
#define _Distouch_OPT8X16_LINE_8_Y1_		(_LCD_YSIZE_ / 8) * 7
#define _Distouch_OPT8X16_LINE_8_Y2_		(_LCD_YSIZE_ / 8) * 8

/* 16X22 OPT */
#define _Distouch_OPT16X22_LINE_1_X1_		0
#define _Distouch_OPT16X22_LINE_1_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_1_Y1_		(_LCD_YSIZE_ / 16) * 0
#define _Distouch_OPT16X22_LINE_1_Y2_		(_LCD_YSIZE_ / 16) * 1

#define _Distouch_OPT16X22_LINE_2_X1_		0
#define _Distouch_OPT16X22_LINE_2_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_2_Y1_		(_LCD_YSIZE_ / 16) * 1
#define _Distouch_OPT16X22_LINE_2_Y2_		(_LCD_YSIZE_ / 16) * 2

#define _Distouch_OPT16X22_LINE_3_X1_		0
#define _Distouch_OPT16X22_LINE_3_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_3_Y1_		(_LCD_YSIZE_ / 16) * 2
#define _Distouch_OPT16X22_LINE_3_Y2_		(_LCD_YSIZE_ / 16) * 3

#define _Distouch_OPT16X22_LINE_4_X1_		0
#define _Distouch_OPT16X22_LINE_4_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_4_Y1_		(_LCD_YSIZE_ / 16) * 3
#define _Distouch_OPT16X22_LINE_4_Y2_		(_LCD_YSIZE_ / 16) * 4

#define _Distouch_OPT16X22_LINE_5_X1_		0
#define _Distouch_OPT16X22_LINE_5_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_5_Y1_		(_LCD_YSIZE_ / 16) * 4
#define _Distouch_OPT16X22_LINE_5_Y2_		(_LCD_YSIZE_ / 16) * 5

#define _Distouch_OPT16X22_LINE_6_X1_		0
#define _Distouch_OPT16X22_LINE_6_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_6_Y1_		(_LCD_YSIZE_ / 16) * 5
#define _Distouch_OPT16X22_LINE_6_Y2_		(_LCD_YSIZE_ / 16) * 6

#define _Distouch_OPT16X22_LINE_7_X1_		0
#define _Distouch_OPT16X22_LINE_7_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_7_Y1_		(_LCD_YSIZE_ / 16) * 6
#define _Distouch_OPT16X22_LINE_7_Y2_		(_LCD_YSIZE_ / 16) * 7

#define _Distouch_OPT16X22_LINE_8_X1_		0
#define _Distouch_OPT16X22_LINE_8_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_8_Y1_		(_LCD_YSIZE_ / 16) * 7
#define _Distouch_OPT16X22_LINE_8_Y2_		(_LCD_YSIZE_ / 16) * 8

#define _Distouch_OPT16X22_LINE_9_X1_		0
#define _Distouch_OPT16X22_LINE_9_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_9_Y1_		(_LCD_YSIZE_ / 16) * 8
#define _Distouch_OPT16X22_LINE_9_Y2_		(_LCD_YSIZE_ / 16) * 9

#define _Distouch_OPT16X22_LINE_10_X1_		0
#define _Distouch_OPT16X22_LINE_10_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_10_Y1_		(_LCD_YSIZE_ / 16) * 9
#define _Distouch_OPT16X22_LINE_10_Y2_		(_LCD_YSIZE_ / 16) * 10

#define _Distouch_OPT16X22_LINE_11_X1_		0
#define _Distouch_OPT16X22_LINE_11_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_11_Y1_		(_LCD_YSIZE_ / 16) * 10
#define _Distouch_OPT16X22_LINE_11_Y2_		(_LCD_YSIZE_ / 16) * 11

#define _Distouch_OPT16X22_LINE_12_X1_		0
#define _Distouch_OPT16X22_LINE_12_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_12_Y1_		(_LCD_YSIZE_ / 16) * 11
#define _Distouch_OPT16X22_LINE_12_Y2_		(_LCD_YSIZE_ / 16) * 12

#define _Distouch_OPT16X22_LINE_13_X1_		0
#define _Distouch_OPT16X22_LINE_13_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_13_Y1_		(_LCD_YSIZE_ / 16) * 12
#define _Distouch_OPT16X22_LINE_13_Y2_		(_LCD_YSIZE_ / 16) * 13

#define _Distouch_OPT16X22_LINE_14_X1_		0
#define _Distouch_OPT16X22_LINE_14_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_14_Y1_		(_LCD_YSIZE_ / 16) * 13
#define _Distouch_OPT16X22_LINE_14_Y2_		(_LCD_YSIZE_ / 16) * 14

#define _Distouch_OPT16X22_LINE_15_X1_		0
#define _Distouch_OPT16X22_LINE_15_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_15_Y1_		(_LCD_YSIZE_ / 16) * 14
#define _Distouch_OPT16X22_LINE_15_Y2_		(_LCD_YSIZE_ / 16) * 15

#define _Distouch_OPT16X22_LINE_16_X1_		0
#define _Distouch_OPT16X22_LINE_16_X2_		_LCD_XSIZE_
#define _Distouch_OPT16X22_LINE_16_Y1_		(_LCD_YSIZE_ / 16) * 15
#define _Distouch_OPT16X22_LINE_16_Y2_		(_LCD_YSIZE_ / 16) * 16

/* _CUSTReceipt_ */
#define _Distouch_CUSTReceipt_ENTER_X1_		((90	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_CUSTReceipt_ENTER_X2_		((170	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_CUSTReceipt_ENTER_Y1_		((370	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_CUSTReceipt_ENTER_Y2_		((410	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_CUSTReceipt_CANCEL_X1_	((220	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_CUSTReceipt_CANCEL_X2_	((300	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
/*  修改有取消按鍵的範圍過大問題 2019/10/8 [SAM] */
#define _Distouch_CUSTReceipt_CANCEL_Y1_	((370	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_CUSTReceipt_CANCEL_Y2_	((410	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* _CHECK_PWD_EDIT_ */
#define _Distouch_CHECKPWDEdit_ENTER_X1_	((200	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_CHECKPWDEdit_ENTER_X2_	((275	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_CHECKPWDEdit_ENTER_Y1_	((310	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_CHECKPWDEdit_ENTER_Y2_	((350	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* HG_SELECT */
#define	_Distouch_HG_SELECT_Touch_YES_X1_	((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define	_Distouch_HG_SELECT_Touch_YES_X2_	((245	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define	_Distouch_HG_SELECT_Touch_YES_Y1_	((325	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define	_Distouch_HG_SELECT_Touch_YES_Y2_	((465	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define	_Distouch_HG_SELECT_Touch_NO_X1_	((170	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define	_Distouch_HG_SELECT_Touch_NO_X2_	((310	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define	_Distouch_HG_SELECT_Touch_NO_Y1_	((325	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define	_Distouch_HG_SELECT_Touch_NO_Y2_	((465	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* SIGN_CHECK */
#define _Distouch_SIGN_CHECK_KEY_1_X1_		((160	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_SIGN_CHECK_KEY_1_X2_		((205	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_SIGN_CHECK_KEY_1_Y1_		((310	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_SIGN_CHECK_KEY_1_Y2_		((410	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_SIGN_CHECK_KEY_2_X1_		((240	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_SIGN_CHECK_KEY_2_X2_		((310	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_SIGN_CHECK_KEY_2_Y1_		((435	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_SIGN_CHECK_KEY_2_Y2_		((465	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* NOSIGN_CHECK */
#define _Distouch_NOSIGN_CHECK_KEY_1_X1_	((235	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_NOSIGN_CHECK_KEY_1_X2_	((280	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_NOSIGN_CHECK_KEY_1_Y1_	((380	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_NOSIGN_CHECK_KEY_1_Y2_	((410	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_NOSIGN_CHECK_KEY_2_X1_	((240	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_NOSIGN_CHECK_KEY_2_X2_	((310	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_NOSIGN_CHECK_KEY_2_Y1_	((435	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_NOSIGN_CHECK_KEY_2_Y2_	((465	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* NEWUI */
#define _Distouch_NEWUI_IDLE_KEY_FUNCTIONKEY_X1_	0
#define _Distouch_NEWUI_IDLE_KEY_FUNCTIONKEY_X2_	_LCD_XSIZE_
#define _Distouch_NEWUI_IDLE_KEY_FUNCTIONKEY_Y1_	0
#define _Distouch_NEWUI_IDLE_KEY_FUNCTIONKEY_Y2_	_LCD_YSIZE_

/* FUNC_NEWUI_LINE_3_TO_8_3X3_ */
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_1_X1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_1_X2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_1_Y1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_1_Y2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_2_X1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_2_X2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_2_Y1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_2_Y2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_3_X1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_3_X2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_3_Y1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_3_Y2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_4_X1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_4_X2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_4_Y1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_4_Y2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_5_X1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_5_X2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_5_Y1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_5_Y2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_6_X1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_6_X2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_6_Y1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_6_Y2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_7_X1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_7_X2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_7_Y1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_7_Y2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_8_X1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_8_X2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_8_Y1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_8_Y2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_9_X1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_9_X2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_9_Y1_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_9_Y2_	_DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y2_

/* FUNC_NEWUI_LINE_3_TO_7_3X3_ */
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_1_X1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN1_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_1_X2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN1_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_1_Y1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_1_Y2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_2_X1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN2_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_2_X2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN2_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_2_Y1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_2_Y2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_3_X1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN3_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_3_X2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN3_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_3_Y1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_3_Y2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_4_X1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN1_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_4_X2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN1_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_4_Y1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_4_Y2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_5_X1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN2_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_5_X2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN2_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_5_Y1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_5_Y2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_6_X1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN3_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_6_X2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN3_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_6_Y1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_6_Y2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_7_X1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN1_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_7_X2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN1_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_7_Y1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_7_Y2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_8_X1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN2_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_8_X2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN2_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_8_Y1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_8_Y2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y2_

#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_9_X1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN3_X1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_9_X2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_COLUMN3_X2_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_9_Y1_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y1_
#define _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_9_Y2_	_DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y2_

/* FUNC_NEWUI_LINE_2_TO_7_3X4_ */
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_X1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN1_X1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_X2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN1_X2_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE1_Y1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE1_Y2_

#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_2_X1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN2_X1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_2_X2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN2_X2_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_2_Y1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE1_Y1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_2_Y2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE1_Y2_

#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_3_X1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN3_X1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_3_X2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN3_X2_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_3_Y1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE1_Y1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_3_Y2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE1_Y2_

#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_X1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN1_X1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_X2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN1_X2_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE2_Y1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE2_Y2_

#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_X1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN2_X1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_X2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN2_X2_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_Y1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE2_Y1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_Y2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE2_Y2_

#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_6_X1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN3_X1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_6_X2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN3_X2_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_6_Y1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE2_Y1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_6_Y2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE2_Y2_

#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_X1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN1_X1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_X2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN1_X2_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE3_Y1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE3_Y2_

#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_X1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN2_X1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_X2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN2_X2_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_Y1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE3_Y1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_Y2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE3_Y2_

#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_9_X1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN3_X1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_9_X2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN3_X2_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_9_Y1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE3_Y1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_9_Y2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE3_Y2_

#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_10_X1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN1_X1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_10_X2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN1_X2_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_10_Y1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE4_Y1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_10_Y2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE4_Y2_

#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_X1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN2_X1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_X2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN2_X2_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE4_Y1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE4_Y2_

#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_12_X1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN3_X1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_12_X2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_COLUMN3_X2_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_12_Y1_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE4_Y1_
#define _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_12_Y2_	_DisTouch_NEWUI_FUNC_2_TO_7_3X4_LINE4_Y2_


#define _Distouch_NEWUI_LAST_PAGE_BUTTON_X1_		((40	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_NEWUI_LAST_PAGE_BUTTON_X2_		((140	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_NEWUI_LAST_PAGE_BUTTON_Y1_		((420	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_NEWUI_LAST_PAGE_BUTTON_Y2_		((480	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define	_Distouch_NEWUI_RETURN_IDLE_BUTTON_X1_		((180	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define	_Distouch_NEWUI_RETURN_IDLE_BUTTON_X2_		((280	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define	_Distouch_NEWUI_RETURN_IDLE_BUTTON_Y1_		((420	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define	_Distouch_NEWUI_RETURN_IDLE_BUTTON_Y2_		((480	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_NEWUI_CHOOSE_HOST_HOST_1_X1_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN1_X1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_1_X2_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN1_X2_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_1_Y1_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE1_Y1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_1_Y2_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE1_Y2_

#define _Distouch_NEWUI_CHOOSE_HOST_HOST_2_X1_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN2_X1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_2_X2_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN2_X2_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_2_Y1_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE1_Y1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_2_Y2_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE1_Y2_

#define _Distouch_NEWUI_CHOOSE_HOST_HOST_3_X1_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN3_X1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_3_X2_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN3_X2_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_3_Y1_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE1_Y1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_3_Y2_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE1_Y2_

#define _Distouch_NEWUI_CHOOSE_HOST_HOST_4_X1_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN1_X1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_4_X2_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN1_X2_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_4_Y1_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE2_Y1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_4_Y2_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE2_Y2_

#define _Distouch_NEWUI_CHOOSE_HOST_HOST_5_X1_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN2_X1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_5_X2_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN2_X1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_5_Y1_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE2_Y1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_5_Y2_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE2_Y2_

#define _Distouch_NEWUI_CHOOSE_HOST_HOST_6_X1_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN3_X1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_6_X2_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_COLUMN3_X2_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_6_Y1_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE2_Y1_
#define _Distouch_NEWUI_CHOOSE_HOST_HOST_6_Y2_	_DisTouch_NEWUI_CHOOSE_HOST_2X3_LINE2_Y2_

#define _Distouch_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_X1_		((40	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_X2_		((140	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)	
#define _Distouch_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_Y1_		((420	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_Y2_		((470	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_X1_	((180	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_X2_	((280	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_Y1_	((420	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_Y2_	((470	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)


#define _Distouch_NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_X1_	_Distouch_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_X1_ 	
#define _Distouch_NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_X2_	_Distouch_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_X2_
#define _Distouch_NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_Y1_	_Distouch_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_Y1_
#define _Distouch_NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_Y2_	_Distouch_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_Y2_

#define _Distouch_NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_X1_	_Distouch_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_X1_
#define _Distouch_NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_X2_	_Distouch_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_X2_
#define _Distouch_NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_Y1_	_Distouch_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_Y1_
#define _Distouch_NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_Y2_	_Distouch_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_Y2_

#define _Distouch_BATCH_END_Touch_ENTER_BUTTON_X1_			((140	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_BATCH_END_Touch_ENTER_BUTTON_X2_			((215	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_BATCH_END_Touch_ENTER_BUTTON_Y1_			((360	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_BATCH_END_Touch_ENTER_BUTTON_Y2_			((430	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_TOUCH_OX_LINE8_8_ENTER_BUTTON_X1_			((85	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TOUCH_OX_LINE8_8_ENTER_BUTTON_X2_			((150	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TOUCH_OX_LINE8_8_ENTER_BUTTON_Y1_			((435	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_TOUCH_OX_LINE8_8_ENTER_BUTTON_Y2_			((465	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_TOUCH_OX_LINE8_8_CANCEL_BUTTON_X1_			((205	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TOUCH_OX_LINE8_8_CANCEL_BUTTON_X2_			((270	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TOUCH_OX_LINE8_8_CANCEL_BUTTON_Y1_			((435	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_TOUCH_OX_LINE8_8_CANCEL_BUTTON_Y2_			((465	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_TOUCH_O_LINE8_8_ENTER_BUTTON_X1_			((145	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TOUCH_O_LINE8_8_ENTER_BUTTON_X2_			((215	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TOUCH_O_LINE8_8_ENTER_BUTTON_Y1_			((430	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_TOUCH_O_LINE8_8_ENTER_BUTTON_Y2_			((465	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _Distouch_TOUCH_X_LINE8_8_CANCEL_BUTTON_X1_			((145	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TOUCH_X_LINE8_8_CANCEL_BUTTON_X2_			((215	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TOUCH_X_LINE8_8_CANCEL_BUTTON_Y1_			((430	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_TOUCH_X_LINE8_8_CANCEL_BUTTON_Y2_			((465	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)


#define _Distouch_TOUCH_ENTER_X1			((95	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TOUCH_ENTER_X2			((225	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _Distouch_TOUCH_ENTER_Y1			((395	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _Distouch_TOUCH_ENTER_Y2			((450	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* 座標值 END */


/* 判斷值 */
/* 拖曳起始點到結尾點，判斷成功需要的距離，值越小，越易判定翻頁成功 */
#define _DisTouch_Judgement_Horizontal_Distance_	((30	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _DisTouch_Judgement_Vertical_Distance_		((30	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* DISTOUCH_OBJECT用 Start */
#define	_DisTouch_PenStatus_Up_		1
#define	_DisTouch_PenStatus_Down_	2
/* DISTOUCH_OBJECT用 End */

#define _DisTouch_Event_Num_		30

/* 事件發生時間的結構 */
typedef struct 
{
	long	lnTime;			/* Seconds since the Epoch.  在計算機內部，時間以紀元以來的秒數表示，在 GNU 和 POSIXPOSIX 系统上，纪元為 1970-01-01 00:00:00 UTC */
	long	lnMicroSecond;		/* Signed count of microseconds.  */
} DISTOUCH_TIMEVAL;

/* 紀錄觸控事件的結構 */
typedef struct 
{
	DISTOUCH_TIMEVAL	srTime;		/* 按下去的時間點 Time Stamp */
	unsigned short		ushType;	/* 事件類型 參照linux/input.h中的Event types部份 */
	unsigned short		ushCode;	/* 事件屬性 事件中細項的屬性如ABS_X、ABS_Y */
	unsigned int		uinValue;	/* 屬性的值 */
} DISTOUCH_EVENT;

/* 紀錄事件座標的結構 */
typedef struct 
{
	int		inX_Temp;	/* 事件的X座標值 */
	int		inY_Temp;	/* 事件的Y座標值 */
	int		inXpen_up;	/* 事件的X起始座標值 */
	int		inYpen_up;	/* 事件的Y起始座標值 */
	int		inXpen_down;	/* 事件的X結束座標值 */
	int		inYpen_down;	/* 事件的Y起始座標值 */
	int		inPenStatus;	/* 1表示PenUp，2表示PenDown */
	unsigned char	uszClicking;	/* 表示是否要判斷點擊，從Pen Down到第一次SYN判斷為點擊，若超過拖曳距離，也不判斷點擊 */
} DISTOUCH_OBJECT;

int inDisTouch_Open_TouchFile(int* inTouch_Handle);
int inDisTouch_Close_TouchFile(int *inTouch_Handle);
int inDisTouch_Read_TouchFile(int *inTouch_Handle);
int inDisTouch_Flush_TouchFile(void);
int inDisTouch_Sensor(int inTouchSensorFunc);
int inDisTouch_InArea(DISTOUCH_OBJECT *srObj, int inTouchSensorFunc);
int inDisTouch_IsSigned(DISTOUCH_OBJECT *srDisTouchObj, int inTouchSensorFunc);
int inDisTouch_TouchSensor_Click_Slide(int inTouchSensorFunc);
int inDisTouch_IsSlidePage(DISTOUCH_OBJECT *srDisTouchObj, int inTouchSensorFunc);
int inDisTouch_Reverse_Back_Area(unsigned short usX, unsigned short usY, unsigned short usXSize, unsigned short usYSize);
int inDisTouch_Reverse_Back_Key(int inTouchSensorFunc, int inEvent);
