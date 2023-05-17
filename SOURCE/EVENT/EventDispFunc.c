

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"

#include "../FUNCTION/Function.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/TDT.h"
#include "../FUNCTION/EDC.h"

#include "MenuMsg.h"

#include "EventDispFunc.h"

extern int ginMachineType;

/*
Function        : inEVENT_DSIP_DecideDisplayIdleImage
Date&Time   : 2019/03/13
Describe        : 顯示主畫面 的感應圖示 信用卡:70 * 48 票證:70 * 48
 */
int inEVENT_DSIP_DecideDisplayIdleImage() {
	int inLogLin;
	int inCreditIndex = 0;
	int inTicketIndex = 0;
	int inCreditLine1 = 0;
	int inCreditLine2 = 0;
	int inTicketLine1 = 0;
	int inTicketLine2 = 0;
	char szFESMode[2 + 1] = {0};
	char szPic[10][100 + 1] = {};
	char szPic_Ticket[8][100 + 1] = {};
	char szFunEnable[2 + 1] = {0};
	char szFunEnable2[2 + 1] = {0};
	char szTMSOK[2 + 1] = {0};

	memset(szFESMode, 0x00, sizeof (szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szTMSOK, 0x00, sizeof (szTMSOK));
	inGetTMSOK(szTMSOK);

	inDISP_ClearAll();
	if (ginMachineType == _CASTLE_TYPE_MP200_) 
	{
		if (1) 
		{
			inDISP_PutGraphic(_TOUCH_NEWUI_IDLE_NO_AMOUNT_, 0, _COORDINATE_Y_LINE_8_1_);
			inDISP_PutGraphic(_MENU_HOST_1099_NEXSYS_BIG_, 0, _COORDINATE_Y_LINE_16_2_);
		} else {
			inDISP_PutGraphic(_TOUCH_NEWUI_IDLE_AMOUNT_, 0, _COORDINATE_Y_LINE_8_1_);
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
		}
	} else if (ginMachineType == _CASTLE_TYPE_V3UL_) 
	{
		inDISP_PutGraphic(_TOUCH_NEWUI_IDLE_NO_AMOUNT_, 0, _COORDINATE_Y_LINE_8_1_);
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
	} else if (ginMachineType == _CASTLE_TYPE_UPT1000_) /* 因為不能輸入金額，所以修改圖片 2019/9/26 下午 4:08 [SAM]*/ 
	{
		inDISP_PutGraphic(_TOUCH_UPT_NEWUI_IDLE_NO_AMOUNT_, 0, _COORDINATE_Y_LINE_8_1_);
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
	} else 
	{
		inDISP_PutGraphic(_TOUCH_NEWUI_IDLE_AMOUNT_, 0, _COORDINATE_Y_LINE_8_1_);
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
	}

	inDISP_LogPrintfWithFlag("Decide_Display_Idle TSMOK  [%s]", szTMSOK);

	if (memcmp(szTMSOK, "Y", strlen("Y")) == 0) 
	{
		if (ginMachineType == _CASTLE_TYPE_MP200_) 
		{
			memset(szPic, 0x00, sizeof (szPic));

			/* 順序為VISA、MASTERCARD、JCB、UNIONPAY、AE、DISCOVER、SMARTPAY */
			/* VISA */
			memset(szFunEnable, 0x00, sizeof (szFunEnable));
			inGetVISAPaywaveEnable(szFunEnable);
			if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) 
			{
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_VISA_BIG_, strlen(_MENU_CARD_LOGO_VISA_BIG_));
				inCreditIndex++;
			}

			/* MASTERCARD */
			memset(szFunEnable, 0x00, sizeof (szFunEnable));
			inGetMCPaypassEnable(szFunEnable);
			if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) 
			{
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_MASTERCARD_BIG_, strlen(_MENU_CARD_LOGO_MASTERCARD_BIG_));
				inCreditIndex++;
			}

			/* JCB */
			memset(szFunEnable, 0x00, sizeof (szFunEnable));
			inGetJCBJspeedyEnable(szFunEnable);
			if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) 
			{
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_JCB_BIG_, strlen(_MENU_CARD_LOGO_JCB_BIG_));
				inCreditIndex++;
			}

			/* UnionPay */
			memset(szFunEnable, 0x00, sizeof (szFunEnable));
			inGetCUPContactlessEnable(szFunEnable);
			memset(szFunEnable2, 0x00, sizeof (szFunEnable2));
			inGetCUPFuncEnable(szFunEnable2);
			if ((memcmp(szFunEnable, "Y", strlen("Y")) == 0) && (memcmp(szFunEnable2, "Y", strlen("Y")) == 0))
			{
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_CUP_BIG_, strlen(_MENU_CARD_LOGO_CUP_BIG_));
				inCreditIndex++;
			}

			/* AE */
			memset(szFunEnable, 0x00, sizeof (szFunEnable));
			inGetAMEXContactlessEnable(szFunEnable);
			if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
			{
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_AMEX_BIG_, strlen(_MENU_CARD_LOGO_AMEX_BIG_));
				inCreditIndex++;
			}

			/* DISCOVER */
			memset(szFunEnable, 0x00, sizeof (szFunEnable));
			inGetDFS_Contactless_Enable(szFunEnable);
			if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) 
			{
				/* DINERS */
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_DINERS_BIG_, strlen(_MENU_CARD_LOGO_DINERS_BIG_));
				inCreditIndex++;

				/* DISCOVER */
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_DISCOVER_BIG_, strlen(_MENU_CARD_LOGO_DISCOVER_BIG_));
				inCreditIndex++;
			}
			
			/* SMARTPAY */
			memset(szFunEnable, 0x00, sizeof (szFunEnable));
			inGetSmartPayContactlessEnable(szFunEnable);
			memset(szFunEnable2, 0x00, sizeof (szFunEnable2));
			inGetFiscFuncEnable(szFunEnable2);
			if ((memcmp(szFunEnable, "Y", strlen("Y")) == 0) && (memcmp(szFunEnable2, "Y", strlen("Y")) == 0))
			{
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_SMARTPAY_BIG_, strlen(_MENU_CARD_LOGO_SMARTPAY_BIG_));
				inCreditIndex++;
			}

			/* 決定圖片放置行數 */
			inCreditLine1 = _COORDINATE_Y_CTLS_LOGO_BIG_Line_1_;
			inCreditLine2 = _COORDINATE_Y_CTLS_LOGO_BIG_Line_2_;
			/* 少於等於5個LOGO時，只有一行 */
			if (inCreditIndex <= 5) {
				inTicketLine1 = _COORDINATE_Y_CTLS_LOGO_BIG_Line_2_;
				inTicketLine2 = _COORDINATE_Y_CTLS_LOGO_BIG_Line_3_;
			} else {
				inTicketLine1 = _COORDINATE_Y_CTLS_LOGO_BIG_Line_3_;
				inTicketLine2 = _COORDINATE_Y_CTLS_LOGO_BIG_Line_4_;
			}

			/* 信用卡只有一張圖 */
			if (inCreditIndex == 1) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inCreditLine1);
				}
			} else if (inCreditIndex == 2) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_4_2_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_4_3_, inCreditLine1);
				}
			} else if (inCreditIndex == 3) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_3_2_, inCreditLine1);
				}

				if (strlen(&szPic[2][0]) != 0) {
					inDISP_PutGraphic(&szPic[2][0], _COORDINATE_X_CTLS_LOGO_3_3_, inCreditLine1);
				}
			} else if (inCreditIndex == 4) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_4_1_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_4_2_, inCreditLine1);
				}

				if (strlen(&szPic[2][0]) != 0) {
					inDISP_PutGraphic(&szPic[2][0], _COORDINATE_X_CTLS_LOGO_4_3_, inCreditLine1);
				}

				if (strlen(&szPic[3][0]) != 0) {
					inDISP_PutGraphic(&szPic[3][0], _COORDINATE_X_CTLS_LOGO_4_4_, inCreditLine1);
				}
			} else if (inCreditIndex == 5) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_5_1_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_5_2_, inCreditLine1);
				}

				if (strlen(&szPic[2][0]) != 0) {
					inDISP_PutGraphic(&szPic[2][0], _COORDINATE_X_CTLS_LOGO_5_3_, inCreditLine1);
				}

				if (strlen(&szPic[3][0]) != 0) {
					inDISP_PutGraphic(&szPic[3][0], _COORDINATE_X_CTLS_LOGO_5_4_, inCreditLine1);
				}

				if (strlen(&szPic[4][0]) != 0) {
					inDISP_PutGraphic(&szPic[4][0], _COORDINATE_X_CTLS_LOGO_5_5_, inCreditLine1);
				}

			} else if (inCreditIndex == 6) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_3_2_, inCreditLine1);
				}

				if (strlen(&szPic[2][0]) != 0) {
					inDISP_PutGraphic(&szPic[2][0], _COORDINATE_X_CTLS_LOGO_3_3_, inCreditLine1);
				}

				if (strlen(&szPic[3][0]) != 0) {
					inDISP_PutGraphic(&szPic[3][0], _COORDINATE_X_CTLS_LOGO_3_1_, inCreditLine2);
				}

				if (strlen(&szPic[4][0]) != 0) {
					inDISP_PutGraphic(&szPic[4][0], _COORDINATE_X_CTLS_LOGO_3_2_, inCreditLine2);
				}

				if (strlen(&szPic[5][0]) != 0) {
					inDISP_PutGraphic(&szPic[5][0], _COORDINATE_X_CTLS_LOGO_3_3_, inCreditLine2);
				}

			} else if (inCreditIndex == 7) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_3_2_, inCreditLine1);
				}

				if (strlen(&szPic[2][0]) != 0) {
					inDISP_PutGraphic(&szPic[2][0], _COORDINATE_X_CTLS_LOGO_3_3_, inCreditLine1);
				}

				if (strlen(&szPic[3][0]) != 0) {
					inDISP_PutGraphic(&szPic[3][0], _COORDINATE_X_CTLS_LOGO_4_1_, inCreditLine2);
				}

				if (strlen(&szPic[4][0]) != 0) {
					inDISP_PutGraphic(&szPic[4][0], _COORDINATE_X_CTLS_LOGO_4_2_, inCreditLine2);
				}

				if (strlen(&szPic[5][0]) != 0) {
					inDISP_PutGraphic(&szPic[5][0], _COORDINATE_X_CTLS_LOGO_4_3_, inCreditLine2);
				}

				if (strlen(&szPic[6][0]) != 0) {
					inDISP_PutGraphic(&szPic[6][0], _COORDINATE_X_CTLS_LOGO_4_4_, inCreditLine2);
				}
			} else if (inCreditIndex == 8) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_4_1_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_4_2_, inCreditLine1);
				}

				if (strlen(&szPic[2][0]) != 0) {
					inDISP_PutGraphic(&szPic[2][0], _COORDINATE_X_CTLS_LOGO_4_3_, inCreditLine1);
				}

				if (strlen(&szPic[3][0]) != 0) {
					inDISP_PutGraphic(&szPic[3][0], _COORDINATE_X_CTLS_LOGO_4_4_, inCreditLine1);
				}

				if (strlen(&szPic[4][0]) != 0) {
					inDISP_PutGraphic(&szPic[4][0], _COORDINATE_X_CTLS_LOGO_4_1_, inCreditLine2);
				}

				if (strlen(&szPic[5][0]) != 0) {
					inDISP_PutGraphic(&szPic[5][0], _COORDINATE_X_CTLS_LOGO_4_2_, inCreditLine2);
				}

				if (strlen(&szPic[6][0]) != 0) {
					inDISP_PutGraphic(&szPic[6][0], _COORDINATE_X_CTLS_LOGO_4_3_, inCreditLine2);
				}
				if (strlen(&szPic[7][0]) != 0) {
					inDISP_PutGraphic(&szPic[7][0], _COORDINATE_X_CTLS_LOGO_4_4_, inCreditLine2);
				}
			}

			/* 票證只有一張圖 */
			if (inTicketIndex == 1) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inTicketLine1);
				}
			} else if (inTicketIndex == 2) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_4_2_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[1][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[1][0], _COORDINATE_X_CTLS_LOGO_4_3_, inTicketLine1);
				}
			} else if (inTicketIndex == 3) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[1][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[1][0], _COORDINATE_X_CTLS_LOGO_3_2_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[2][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[2][0], _COORDINATE_X_CTLS_LOGO_3_3_, inTicketLine1);
				}
			} else if (inTicketIndex == 4) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_4_1_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[1][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[1][0], _COORDINATE_X_CTLS_LOGO_4_2_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[2][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[2][0], _COORDINATE_X_CTLS_LOGO_4_3_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[3][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[3][0], _COORDINATE_X_CTLS_LOGO_4_4_, inTicketLine1);
				}
			} else if (inTicketIndex == 5) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_5_1_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[1][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[1][0], _COORDINATE_X_CTLS_LOGO_5_2_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[2][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[2][0], _COORDINATE_X_CTLS_LOGO_5_3_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[3][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[3][0], _COORDINATE_X_CTLS_LOGO_5_4_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[4][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[4][0], _COORDINATE_X_CTLS_LOGO_5_5_, inTicketLine1);
				}

			} else if (inTicketIndex == 6) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[1][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[1][0], _COORDINATE_X_CTLS_LOGO_3_2_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[2][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[2][0], _COORDINATE_X_CTLS_LOGO_3_3_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[3][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[3][0], _COORDINATE_X_CTLS_LOGO_3_1_, inTicketLine2);
				}

				if (strlen(&szPic_Ticket[4][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[4][0], _COORDINATE_X_CTLS_LOGO_3_2_, inTicketLine2);
				}

				if (strlen(&szPic_Ticket[5][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[5][0], _COORDINATE_X_CTLS_LOGO_3_3_, inTicketLine2);
				}

			} else if (inTicketIndex == 7) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[1][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[1][0], _COORDINATE_X_CTLS_LOGO_3_2_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[2][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[2][0], _COORDINATE_X_CTLS_LOGO_3_3_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[3][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[3][0], _COORDINATE_X_CTLS_LOGO_4_1_, inTicketLine2);
				}

				if (strlen(&szPic_Ticket[4][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[4][0], _COORDINATE_X_CTLS_LOGO_4_2_, inTicketLine2);
				}

				if (strlen(&szPic_Ticket[5][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[5][0], _COORDINATE_X_CTLS_LOGO_4_3_, inTicketLine2);
				}

				if (strlen(&szPic_Ticket[6][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[6][0], _COORDINATE_X_CTLS_LOGO_4_4_, inTicketLine2);
				}
			}
		} else if (ginMachineType == _CASTLE_TYPE_V3UL_) 
		{
			memset(szPic, 0x00, sizeof (szPic));

			/* 順序為VISA、MASTERCARD、JCB、UNIONPAY、AE、DISCOVER、SMARTPAY */
			/* VISA */
			memset(szFunEnable, 0x00, sizeof (szFunEnable));
			inGetVISAPaywaveEnable(szFunEnable);
			if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_VISA_BIG_, strlen(_MENU_CARD_LOGO_VISA_BIG_));
				inCreditIndex++;
			}

			/* MASTERCARD */
			memset(szFunEnable, 0x00, sizeof (szFunEnable));
			inGetMCPaypassEnable(szFunEnable);
			if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_MASTERCARD_BIG_, strlen(_MENU_CARD_LOGO_MASTERCARD_BIG_));
				inCreditIndex++;
			}

			/* JCB */
			memset(szFunEnable, 0x00, sizeof (szFunEnable));
			inGetJCBJspeedyEnable(szFunEnable);
			if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_JCB_BIG_, strlen(_MENU_CARD_LOGO_JCB_BIG_));
				inCreditIndex++;
			}

			/* UnionPay */
			memset(szFunEnable, 0x00, sizeof (szFunEnable));
			inGetCUPContactlessEnable(szFunEnable);
			memset(szFunEnable2, 0x00, sizeof (szFunEnable2));
			inGetCUPFuncEnable(szFunEnable2);
			if ((memcmp(szFunEnable, "Y", strlen("Y")) == 0) && (memcmp(szFunEnable2, "Y", strlen("Y")) == 0)) {
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_CUP_BIG_, strlen(_MENU_CARD_LOGO_CUP_BIG_));
				inCreditIndex++;
			}


			/* SMARTPAY */
			memset(szFunEnable, 0x00, sizeof (szFunEnable));
			inGetSmartPayContactlessEnable(szFunEnable);
			memset(szFunEnable2, 0x00, sizeof (szFunEnable2));
			inGetFiscFuncEnable(szFunEnable2);
			if ((memcmp(szFunEnable, "Y", strlen("Y")) == 0) && (memcmp(szFunEnable2, "Y", strlen("Y")) == 0)) {
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_SMARTPAY_BIG_, strlen(_MENU_CARD_LOGO_SMARTPAY_BIG_));
				inCreditIndex++;
			}
			
			/* 決定圖片放置行數 */
			inCreditLine1 = _COORDINATE_Y_CTLS_LOGO_BIG_Line_1_;
			inCreditLine2 = _COORDINATE_Y_CTLS_LOGO_BIG_Line_2_;
			/* 少於等於5個LOGO時，只有一行 */
			if (inCreditIndex <= 5) {
				inTicketLine1 = _COORDINATE_Y_CTLS_LOGO_BIG_Line_2_;
				inTicketLine2 = _COORDINATE_Y_CTLS_LOGO_BIG_Line_3_;
			} else {
				inTicketLine1 = _COORDINATE_Y_CTLS_LOGO_BIG_Line_3_;
				inTicketLine2 = _COORDINATE_Y_CTLS_LOGO_BIG_Line_4_;
			}

			/* 信用卡只有一張圖 */
			if (inCreditIndex == 1) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inCreditLine1);
				}
			} else if (inCreditIndex == 2) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_4_2_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_4_3_, inCreditLine1);
				}
			} else if (inCreditIndex == 3) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_3_2_, inCreditLine1);
				}

				if (strlen(&szPic[2][0]) != 0) {
					inDISP_PutGraphic(&szPic[2][0], _COORDINATE_X_CTLS_LOGO_3_3_, inCreditLine1);
				}
			} else if (inCreditIndex == 4) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_4_1_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_4_2_, inCreditLine1);
				}

				if (strlen(&szPic[2][0]) != 0) {
					inDISP_PutGraphic(&szPic[2][0], _COORDINATE_X_CTLS_LOGO_4_3_, inCreditLine1);
				}

				if (strlen(&szPic[3][0]) != 0) {
					inDISP_PutGraphic(&szPic[3][0], _COORDINATE_X_CTLS_LOGO_4_4_, inCreditLine1);
				}
			} else if (inCreditIndex == 5) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_5_1_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_5_2_, inCreditLine1);
				}

				if (strlen(&szPic[2][0]) != 0) {
					inDISP_PutGraphic(&szPic[2][0], _COORDINATE_X_CTLS_LOGO_5_3_, inCreditLine1);
				}

				if (strlen(&szPic[3][0]) != 0) {
					inDISP_PutGraphic(&szPic[3][0], _COORDINATE_X_CTLS_LOGO_5_4_, inCreditLine1);
				}

				if (strlen(&szPic[4][0]) != 0) {
					inDISP_PutGraphic(&szPic[4][0], _COORDINATE_X_CTLS_LOGO_5_5_, inCreditLine1);
				}

			} else if (inCreditIndex == 6) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_3_2_, inCreditLine1);
				}

				if (strlen(&szPic[2][0]) != 0) {
					inDISP_PutGraphic(&szPic[2][0], _COORDINATE_X_CTLS_LOGO_3_3_, inCreditLine1);
				}

				if (strlen(&szPic[3][0]) != 0) {
					inDISP_PutGraphic(&szPic[3][0], _COORDINATE_X_CTLS_LOGO_3_1_, inCreditLine2);
				}

				if (strlen(&szPic[4][0]) != 0) {
					inDISP_PutGraphic(&szPic[4][0], _COORDINATE_X_CTLS_LOGO_3_2_, inCreditLine2);
				}

				if (strlen(&szPic[5][0]) != 0) {
					inDISP_PutGraphic(&szPic[5][0], _COORDINATE_X_CTLS_LOGO_3_3_, inCreditLine2);
				}

			} else if (inCreditIndex == 7) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_3_2_, inCreditLine1);
				}

				if (strlen(&szPic[2][0]) != 0) {
					inDISP_PutGraphic(&szPic[2][0], _COORDINATE_X_CTLS_LOGO_3_3_, inCreditLine1);
				}

				if (strlen(&szPic[3][0]) != 0) {
					inDISP_PutGraphic(&szPic[3][0], _COORDINATE_X_CTLS_LOGO_4_1_, inCreditLine2);
				}

				if (strlen(&szPic[4][0]) != 0) {
					inDISP_PutGraphic(&szPic[4][0], _COORDINATE_X_CTLS_LOGO_4_2_, inCreditLine2);
				}

				if (strlen(&szPic[5][0]) != 0) {
					inDISP_PutGraphic(&szPic[5][0], _COORDINATE_X_CTLS_LOGO_4_3_, inCreditLine2);
				}

				if (strlen(&szPic[6][0]) != 0) {
					inDISP_PutGraphic(&szPic[6][0], _COORDINATE_X_CTLS_LOGO_4_4_, inCreditLine2);
				}
			} else if (inCreditIndex == 8) {
				if (strlen(&szPic[0][0]) != 0) {
					inDISP_PutGraphic(&szPic[0][0], _COORDINATE_X_CTLS_LOGO_4_1_, inCreditLine1);
				}

				if (strlen(&szPic[1][0]) != 0) {
					inDISP_PutGraphic(&szPic[1][0], _COORDINATE_X_CTLS_LOGO_4_2_, inCreditLine1);
				}

				if (strlen(&szPic[2][0]) != 0) {
					inDISP_PutGraphic(&szPic[2][0], _COORDINATE_X_CTLS_LOGO_4_3_, inCreditLine1);
				}

				if (strlen(&szPic[3][0]) != 0) {
					inDISP_PutGraphic(&szPic[3][0], _COORDINATE_X_CTLS_LOGO_4_4_, inCreditLine1);
				}

				if (strlen(&szPic[4][0]) != 0) {
					inDISP_PutGraphic(&szPic[4][0], _COORDINATE_X_CTLS_LOGO_4_1_, inCreditLine2);
				}

				if (strlen(&szPic[5][0]) != 0) {
					inDISP_PutGraphic(&szPic[5][0], _COORDINATE_X_CTLS_LOGO_4_2_, inCreditLine2);
				}

				if (strlen(&szPic[6][0]) != 0) {
					inDISP_PutGraphic(&szPic[6][0], _COORDINATE_X_CTLS_LOGO_4_3_, inCreditLine2);
				}
				if (strlen(&szPic[7][0]) != 0) {
					inDISP_PutGraphic(&szPic[7][0], _COORDINATE_X_CTLS_LOGO_4_4_, inCreditLine2);
				}
			}

			/* 票證只有一張圖 */
			if (inTicketIndex == 1) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inTicketLine1);
				}
			} else if (inTicketIndex == 2) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_4_2_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[1][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[1][0], _COORDINATE_X_CTLS_LOGO_4_3_, inTicketLine1);
				}
			} else if (inTicketIndex == 3) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[1][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[1][0], _COORDINATE_X_CTLS_LOGO_3_2_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[2][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[2][0], _COORDINATE_X_CTLS_LOGO_3_3_, inTicketLine1);
				}
			} else if (inTicketIndex == 4) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_4_1_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[1][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[1][0], _COORDINATE_X_CTLS_LOGO_4_2_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[2][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[2][0], _COORDINATE_X_CTLS_LOGO_4_3_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[3][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[3][0], _COORDINATE_X_CTLS_LOGO_4_4_, inTicketLine1);
				}
			} else if (inTicketIndex == 5) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_5_1_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[1][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[1][0], _COORDINATE_X_CTLS_LOGO_5_2_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[2][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[2][0], _COORDINATE_X_CTLS_LOGO_5_3_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[3][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[3][0], _COORDINATE_X_CTLS_LOGO_5_4_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[4][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[4][0], _COORDINATE_X_CTLS_LOGO_5_5_, inTicketLine1);
				}

			} else if (inTicketIndex == 6) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[1][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[1][0], _COORDINATE_X_CTLS_LOGO_3_2_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[2][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[2][0], _COORDINATE_X_CTLS_LOGO_3_3_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[3][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[3][0], _COORDINATE_X_CTLS_LOGO_3_1_, inTicketLine2);
				}

				if (strlen(&szPic_Ticket[4][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[4][0], _COORDINATE_X_CTLS_LOGO_3_2_, inTicketLine2);
				}

				if (strlen(&szPic_Ticket[5][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[5][0], _COORDINATE_X_CTLS_LOGO_3_3_, inTicketLine2);
				}

			} else if (inTicketIndex == 7) {
				if (strlen(&szPic_Ticket[0][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[0][0], _COORDINATE_X_CTLS_LOGO_3_1_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[1][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[1][0], _COORDINATE_X_CTLS_LOGO_3_2_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[2][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[2][0], _COORDINATE_X_CTLS_LOGO_3_3_, inTicketLine1);
				}

				if (strlen(&szPic_Ticket[3][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[3][0], _COORDINATE_X_CTLS_LOGO_4_1_, inTicketLine2);
				}

				if (strlen(&szPic_Ticket[4][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[4][0], _COORDINATE_X_CTLS_LOGO_4_2_, inTicketLine2);
				}

				if (strlen(&szPic_Ticket[5][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[5][0], _COORDINATE_X_CTLS_LOGO_4_3_, inTicketLine2);
				}

				if (strlen(&szPic_Ticket[6][0]) != 0) {
					inDISP_PutGraphic(&szPic_Ticket[6][0], _COORDINATE_X_CTLS_LOGO_4_4_, inTicketLine2);
				}
			}
		} else 
		{
			if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0) {
				memset(szPic, 0x00, sizeof (szPic));

				/* 行動支付業者和電票業者之LOGO順序 */
				/* ApplePay、GooglePay、SamsungPay、IPASS、ECC、ICASH、HAPPYCASH */
				/* 目前沒有行動支付業者的開關，直接加三 */
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetContactlessEnable(szFunEnable);
				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
					/* APPLEPAY */
					memcpy(&szPic_Ticket[inTicketIndex][0], _MENU_CARD_LOGO_APPLEPAY_, strlen(_MENU_CARD_LOGO_APPLEPAY_));
					inTicketIndex++;
					/* GOOLEPAY */
					memcpy(&szPic_Ticket[inTicketIndex][0], _MENU_CARD_LOGO_GOOGLEPAY_, strlen(_MENU_CARD_LOGO_GOOGLEPAY_));
					inTicketIndex++;
					/* SAMSUNGPAY*/
					memcpy(&szPic_Ticket[inTicketIndex][0], _MENU_CARD_LOGO_SAMSUNGPAY_, strlen(_MENU_CARD_LOGO_SAMSUNGPAY_));
					inTicketIndex++;
				}

				/* IPASS */
				inLoadTDTRec(_TDT_INDEX_00_IPASS_);
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetTicket_HostEnable(szFunEnable);
				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
					memcpy(&szPic_Ticket[inTicketIndex][0], _MENU_CARD_LOGO_IPASS_, strlen(_MENU_CARD_LOGO_IPASS_));
					inTicketIndex++;
				}

				/* ECC */
				inLoadTDTRec(_TDT_INDEX_01_ECC_);
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetTicket_HostEnable(szFunEnable);
				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
					memcpy(&szPic_Ticket[inTicketIndex][0], _MENU_CARD_LOGO_ECC_, strlen(_MENU_CARD_LOGO_ECC_));
					inTicketIndex++;
				}

				

				/* 決定圖片放置行數 */
				inCreditLine1 = _COORDINATE_Y_CTLS_LOGO_Line_1_;
				inCreditLine2 = _COORDINATE_Y_CTLS_LOGO_Line_2_;
				/* 少於等於4個LOGO時，只有一行 */
				if (inCreditIndex == 0) {
					inTicketLine1 = _COORDINATE_Y_CTLS_LOGO_Line_1_;
					inTicketLine2 = _COORDINATE_Y_CTLS_LOGO_Line_2_;
				} else if (inCreditIndex <= 4) {
					inTicketLine1 = _COORDINATE_Y_CTLS_LOGO_Line_2_;
					inTicketLine2 = _COORDINATE_Y_CTLS_LOGO_Line_3_;
				} else {
					inTicketLine1 = _COORDINATE_Y_CTLS_LOGO_Line_3_;
					inTicketLine2 = _COORDINATE_Y_CTLS_LOGO_Line_4_;
				}
			} else {

				memset(szPic, 0x00, sizeof (szPic));

				/* 順序為VISA、MASTERCARD、JCB、UNIONPAY、AE、DISCOVER、SMARTPAY */
				/* VISA */
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetVISAPaywaveEnable(szFunEnable);
				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
					memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_VISA_, strlen(_MENU_CARD_LOGO_VISA_));
					inCreditIndex++;
				}

				/* MASTERCARD */
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetMCPaypassEnable(szFunEnable);
				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
					memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_MASTERCARD_, strlen(_MENU_CARD_LOGO_MASTERCARD_));
					inCreditIndex++;
				}

				/* JCB */
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetJCBJspeedyEnable(szFunEnable);
				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
					memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_JCB_, strlen(_MENU_CARD_LOGO_JCB_));
					inCreditIndex++;
				}

				/* UnionPay */
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetCUPContactlessEnable(szFunEnable);
				memset(szFunEnable2, 0x00, sizeof (szFunEnable2));
				inGetCUPFuncEnable(szFunEnable2);
				if ((memcmp(szFunEnable, "Y", strlen("Y")) == 0) && (memcmp(szFunEnable2, "Y", strlen("Y")) == 0)) {
					memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_CUP_, strlen(_MENU_CARD_LOGO_CUP_));
					inCreditIndex++;
				}

				/* AE */
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetAMEXContactlessEnable(szFunEnable);
				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
					memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_AMEX_, strlen(_MENU_CARD_LOGO_AMEX_));
					inCreditIndex++;
				}


				/* DISCOVER */
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetDFS_Contactless_Enable(szFunEnable);
				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
					/* DINERS */
					memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_DINERS_, strlen(_MENU_CARD_LOGO_DINERS_));
					inCreditIndex++;

					/* DISCOVER */
					memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_DISCOVER_, strlen(_MENU_CARD_LOGO_DISCOVER_));
					inCreditIndex++;
				}

				/* SMARTPAY */
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetSmartPayContactlessEnable(szFunEnable);
				memset(szFunEnable2, 0x00, sizeof (szFunEnable2));
				inGetFiscFuncEnable(szFunEnable2);
				if ((memcmp(szFunEnable, "Y", strlen("Y")) == 0) && (memcmp(szFunEnable2, "Y", strlen("Y")) == 0)) {
					memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_SMARTPAY_, strlen(_MENU_CARD_LOGO_SMARTPAY_));
					inCreditIndex++;
				}

				/* 行動支付業者和電票業者之LOGO順序 */
				/* ApplePay、GooglePay、SamsungPay、IPASS、ECC、ICASH、HAPPYCASH */
				/* 目前沒有行動支付業者的開關，直接加三 */
				memset(szPic_Ticket, 0x00, sizeof (szPic_Ticket));
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetContactlessEnable(szFunEnable);
				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
					/* APPLEPAY */
					memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_APPLEPAY_, strlen(_MENU_CARD_LOGO_APPLEPAY_));
					inCreditIndex++;
					/* GOOLEPAY */
					memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_GOOGLEPAY_, strlen(_MENU_CARD_LOGO_GOOGLEPAY_));
					inCreditIndex++;
					/* SAMSUNGPAY*/
					memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_SAMSUNGPAY_, strlen(_MENU_CARD_LOGO_SAMSUNGPAY_));
					inCreditIndex++;
				}

				/* GarminPay */
				memcpy(&szPic[inCreditIndex][0], _MENU_CARD_LOGO_GARMINPAY_, strlen(_MENU_CARD_LOGO_GARMINPAY_));
				inCreditIndex++;

				/* IPASS */
				inLoadTDTRec(_TDT_INDEX_00_IPASS_);
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetTicket_HostEnable(szFunEnable);
				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
					memcpy(&szPic_Ticket[inTicketIndex][0], _MENU_CARD_LOGO_IPASS_, strlen(_MENU_CARD_LOGO_IPASS_));
					inTicketIndex++;
				}

				/* ECC */
				inLoadTDTRec(_TDT_INDEX_01_ECC_);
				memset(szFunEnable, 0x00, sizeof (szFunEnable));
				inGetTicket_HostEnable(szFunEnable);
				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
					memcpy(&szPic_Ticket[inTicketIndex][0], _MENU_CARD_LOGO_ECC_, strlen(_MENU_CARD_LOGO_ECC_));
					inTicketIndex++;
				}
				
				inDISP_LogPrintfWithFlag("  Pic Cnt[%d], PicTicket Cnt[%d]", inCreditIndex, inTicketIndex);

				/* 決定圖片放置行數 */
				inCreditLine1 = _COORDINATE_Y_CTLS_LOGO_Line_1_;
				inCreditLine2 = _COORDINATE_Y_CTLS_LOGO_Line_2_;
				/* 少於等於4個LOGO時，只有一行 */
				if (inCreditIndex == 0) {
					inTicketLine1 = _COORDINATE_Y_CTLS_LOGO_Line_1_;
					inTicketLine2 = _COORDINATE_Y_CTLS_LOGO_Line_2_;
				} else if (inCreditIndex <= 4) {
					inTicketLine1 = _COORDINATE_Y_CTLS_LOGO_Line_2_;
					inTicketLine2 = _COORDINATE_Y_CTLS_LOGO_Line_3_;
				} else {
					inTicketLine1 = _COORDINATE_Y_CTLS_LOGO_Line_3_;
					inTicketLine2 = _COORDINATE_Y_CTLS_LOGO_Line_4_;
				}
			}/* FESMODE */

			for (inLogLin = 0; inLogLin <= inCreditIndex; inLogLin++) {
				if (strlen(&szPic[inLogLin][0]) != 0) {

					if (inLogLin < 3)
						inCreditLine1 = _COORDINATE_Y_CTLS_LOGO_Line_1_;
					else if (inLogLin < 6) {
						inCreditLine1 = _COORDINATE_Y_CTLS_LOGO_Line_2_;
					} else if (inLogLin < 9) {
						inCreditLine1 = _COORDINATE_Y_CTLS_LOGO_Line_3_;
					} else {
						inCreditLine1 = _COORDINATE_Y_CTLS_LOGO_Line_3_;
						inDISP_LogPrintfWithFlag("  Over Cnt  [%d]", inLogLin);
					}

					inDISP_LogPrintfWithFlag("  LogLin [%d], inLogLin = [%d]", inLogLin, inLogLin % 3);

					if (inLogLin % 3 == 0)
						inDISP_PutGraphic(&szPic[inLogLin][0], _COORDINATE_X_CTLS_LOGO_3_1_, inCreditLine1);
					else if (inLogLin % 3 == 1)
						inDISP_PutGraphic(&szPic[inLogLin][0], _COORDINATE_X_CTLS_LOGO_3_2_, inCreditLine1);
					else if (inLogLin % 3 == 2)
						inDISP_PutGraphic(&szPic[inLogLin][0], _COORDINATE_X_CTLS_LOGO_3_3_, inCreditLine1);

				}
			}

			inDISP_LogPrintfWithFlag("  LogOnCnt  [%d] ", inLogLin);
		}/* 分機型 */
	}/* TMS是否OK */

	return (VS_SUCCESS);
}

/*
Function        : inEVENT_DSIP_DecideDisplayIdleImageFullScreen
Date&Time   : 2020/12/22
Describe        : 顯示主畫面 的感應圖示,使用整個畫面的圖進行顯示。
 */
int inEVENT_DSIP_DecideDisplayIdleImageFullScreen() {
	inDISP_PutGraphic(_TOUCH_UPT_NEWUI_IDLE_FULL_PICUTRE_, 0, _COORDINATE_Y_LINE_16_1_);
	return (VS_SUCCESS);
}

