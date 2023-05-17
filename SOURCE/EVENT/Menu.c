#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <sqlite3.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"

#include "../FUNCTION/Accum.h"
#include "../FUNCTION/ASMC.h"
#include "../FUNCTION/Card.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/ECR_FUNC/ECR.h"
#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../FUNCTION/MULTI_FUNC/MultiFunc.h"
#include "../FUNCTION/Signpad.h"
#include "../FUNCTION/KMS.h"
#include "../FUNCTION/PowerManagement.h"
#include "../FUNCTION/TDT.h"
#include "../FUNCTION/IPASSDT.h"
#include "../FUNCTION/ECCDT.h"
#include "../COMM/Comm.h"
#include "../COMM/Ethernet.h"
#include "../COMM/WiFi.h"
#include "../../EMVSRC/EMVsrc.h"
#include "../../EMVSRC/EMVxml.h"

#include "../FUNCTION/CREDIT_FUNC/CreditCheckFunc.h"


/* [新增電票悠遊卡功能]  [SAM] */
#include "../../NCCC/NCCCTicketSrc.h"

#include "Event.h"
#include "MenuMsg.h"
#include "Menu.h"
#include "Flow.h"
#include "EventDispFunc.h"

#include "MENUUI/Layer1Menu.h"
#include "MENUUI/Layer4Menu.h"

extern ECR_GASSTATION_GLOBAL_TABLE gsrECR_GASStation_tb;


/* [新增電票悠遊卡功能] 
 *  移除 NCCC 的電票功能
 *  [SAM] 2022/6/13 下午 5:29 */

/* ginIdleDispFlag用來控制什麼時候要display Idle圖片，
 * 因為while迴圈內一直顯示圖片會造成觸控判斷延遲，
 * 一顯示完圖片就OFF，要離開inEVENT_Responder前ON起來 */
int	ginIdleDispFlag;
int	ginIdleMSRStatus, ginIdleICCStatus, ginMenuKeyIn;
int	ginFallback;

extern	int	ginESC_Idle_flag;	/* idle是否上傳ESC */
extern	int	ginMenuKeyIn, ginIdleMSRStatus, ginIdleICCStatus;
extern	int	ginDebug;
extern	int	ginDisplayDebug;
extern	int	ginEngineerDebug;
extern	int	ginMachineType;
extern	char	gszTermVersionID[15 + 1];
extern	unsigned char	guszCTLSInitiOK;

extern int ginEventCode;

#ifdef _LOOP_TEST_
	static long st_lnTestAmt = 1;
#endif

/*
Function		: inMENU_Decide_Idle_Menu
Date&Time	: 2017/9/18 下午 3:25
Describe		: 
 * 這個function是idle畫面，理論上不應該跳出，
 * 所以在這function再包一層while，若其他Idle UI不用，可以註解掉
 * 
*/
int inMENU_Decide_Idle_Menu()
{
	/* 剛開機要顯示Idle圖片 */
	ginIdleDispFlag = VS_TRUE;
	/* 清空過磁卡資料 */
	inCARD_Clean_MSR_Buffer();

	while (1)
	{
#if defined	_LOAD_KEY_AP_
		inMENU_Load_Key_UI();
#else
		if (ginMachineType == _CASTLE_TYPE_V3C_)
		{
			inMENU_New_UI();
		}
		else if (ginMachineType == _CASTLE_TYPE_V3M_)
		{
			inMENU_New_UI();
		}
		else if (ginMachineType == _CASTLE_TYPE_V3P_)
		{
			inMENU_New_UI();
		}
		else if (ginMachineType == _CASTLE_TYPE_V3UL_)
		{
			inMENU_Old_UI_V3UL();
		}
		else if (ginMachineType == _CASTLE_TYPE_MP200_)
		{
			inMENU_New_UI_MP200();
		}
		else if (ginMachineType == _CASTLE_TYPE_UPT1000_)
		{
			inMENU_New_UI();
		}
		else
		{
			inMENU_New_UI();
		}
#endif
	}

	return (VS_SUCCESS);
}

/*
Function		: inMENU_Old_UI_V3UL
Date&Time	: 2017/10/2 下午 3:46
Describe		: 舊版IDLE UI
*/
int inMENU_Old_UI_V3UL()
{
	int	inRetVal = VS_ERROR;
	char	szCTLSEnable[2 + 1];		/* 觸控是否打開 */
	unsigned char		uszKey;
	TRANSACTION_OBJECT 	pobTran;

	if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);

		while (1)
		{
			/* 初始化 */
			ginIdleMSRStatus = VS_FALSE;	/* idle刷卡 */
			ginIdleICCStatus = VS_FALSE;	/* idle插晶片卡 */
			ginMenuKeyIn = VS_FALSE;	/* idle Keyin */

			/* 顯示IDLE畫面控制，一直Display圖片會造成觸控判斷延遲 */
			if (ginIdleDispFlag == VS_TRUE)
			{
#if (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_) && (defined(_ESUN_MAIN_HOST_) )
                                    inEVENT_DSIP_DecideDisplayIdleImageFullScreen();
#else
                                    inEVENT_DSIP_DecideDisplayIdleImage();
#endif
                                    

				/* 已顯示Idle圖，關閉flag */
				ginIdleDispFlag  = VS_FALSE;
			}

			if (memcmp(&szCTLSEnable[0], "Y", 1) != 0 || guszCTLSInitiOK != VS_TRUE)
			{
				/* 未開感應 */
				/* 偵測磁條刷卡 */
				inRetVal = inCARD_MSREvent();

				if (inRetVal == VS_SUCCESS)
				{
					inEVENT_Responder(_SWIPE_EVENT_);
				}
				else if (inRetVal == VS_SWIPE_ERROR)
				{
					inFunc_ResetTitle(&pobTran);
					inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
					/* 刷卡錯誤會蓋idle圖 */
					ginIdleDispFlag = VS_TRUE;
				}

				/* 偵測晶片插卡 */
				inRetVal = inEMV_ICCEvent();
				if (inRetVal == VS_SUCCESS)
				{
					inEVENT_Responder(_EMV_DO_EVENT_);
				}

			}

			/* 偵測多接設備收到資料 */
			if (inMultiFunc_First_Receive_Check() == VS_SUCCESS)
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					char	szDebugMsg[100 + 1];

					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Receive Event");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
				}
				
				{
					inFunc_CalculateRunTimeGlobal_Start();
				}

				/*若有開工程師debug，只建議直接按0進入 */
				if (ginEngineerDebug == VS_TRUE)
				{

				}
				else
				{
					inEVENT_Responder(_MULTIFUNC_SLAVE_EVENT_);
				}
			}

			/* 偵測按鍵觸發 */
			uszKey = 0x00;
			uszKey = uszKBD_Key();
			if (uszKey != 0x00)
			{
				switch (uszKey)
				{
					case _KEY_1_:
					case _KEY_2_:
					case _KEY_3_:
					case _KEY_4_:
					case _KEY_5_:
					case _KEY_6_:
					case _KEY_7_:
					case _KEY_8_:
					case _KEY_9_:
					case _KEY_0_:
					case _KEY_F1_:
					case _KEY_F2_:
					case _KEY_F3_:
					case _KEY_F4_:
					case _KEY_CLEAR_:
					case _KEY_DOT_:
					case _KEY_ENTER_:
					case _KEY_FUNCTION_:
					case _KEY_CANCEL_:
						inEVENT_Responder(uszKey);
						break;
					default:
						break;
				}

			}

		}/* IDLE while迴圈 */
	}

	return (VS_SUCCESS);
}

/*
Function        :inMENU_New_UI_MP200
Date&Time       :2017/10/2 下午 3:46
Describe        :新版IDLE UI
*/
int inMENU_New_UI_MP200()
{
#if	_MACHINE_TYPE_ == _CASTLE_TYPE_MP200_
	int			inRetVal = VS_ERROR;
	char			szCTLSEnable[2 + 1];		/* 觸控是否打開 */
	unsigned char		uszKey;
	unsigned short		usLen = 0;
	TRANSACTION_OBJECT 	pobTran;

	/* 有觸控才要加 */
#ifdef _TOUCH_CAPBILITY_
	int			inFunc = 0;			/* 接收觸控回傳，進哪一個畫面 */
#endif
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);

	while (1)
	{
                /* 初始化 */
                ginIdleMSRStatus = VS_FALSE;	/* idle刷卡 */
                ginIdleICCStatus = VS_FALSE;	/* idle插晶片卡 */
                ginMenuKeyIn = VS_FALSE;	/* idle Keyin */

                /* 顯示IDLE畫面控制，一直Display圖片會造成觸控判斷延遲 */
                if (ginIdleDispFlag == VS_TRUE)
                {
                        inEVENT_DSIP_DecideDisplayIdleImage();

			/* 已顯示Idle圖，關閉flag */
                        ginIdleDispFlag  = VS_FALSE;
                }

#ifdef _TOUCH_CAPBILITY_
                /* IDLE觸控畫面 */
                inFunc = inDISP_DisplayIdleMessage(); /* 回傳IDLE畫面的按鈕選擇 */

                if (inFunc == _IdleTouch_KEY_1_ || inFunc == _IdleTouch_KEY_2_ || inFunc == _IdleTouch_KEY_3_)
                {
                        inEVENT_Responder(inFunc);
                }
#endif

                if (memcmp(&szCTLSEnable[0], "Y", 1) != 0 || guszCTLSInitiOK != VS_TRUE)
                {
                        /* 未開感應 */
                        /* 偵測磁條刷卡 */
			inRetVal = inCARD_MSREvent();

                        if (inRetVal == VS_SUCCESS)
                        {
                                inEVENT_Responder(_SWIPE_EVENT_);
                        }
			else if (inRetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(&pobTran);
				inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
				/* 刷卡錯誤會蓋idle圖 */
				ginIdleDispFlag = VS_TRUE;
			}

                        /* 偵測晶片插卡 */
			inRetVal = inEMV_ICCEvent();
                        if (inRetVal == VS_SUCCESS)
                        {
                                inEVENT_Responder(_EMV_DO_EVENT_);
                        }

                }

                /* 偵測ECR收到資料 */
		if (inECR_Receive_Check(&usLen) == VS_SUCCESS)
            	{
			inEVENT_Responder(_ECR_EVENT_);
		}

		/* 顯示狀態 */
		inFunc_Display_All_Status_By_Machine_Type();

		/* 偵測按鍵觸發 */
		uszKey = 0x00;
		uszKey = uszKBD_Key();
                if (uszKey != 0x00)
                {
                        switch (uszKey)
                        {
                                case _KEY_1_:
                                case _KEY_2_:
                                case _KEY_3_:
                                case _KEY_4_:
                                case _KEY_5_:
                                case _KEY_6_:
                                case _KEY_7_:
                                case _KEY_8_:
                                case _KEY_9_:
                                case _KEY_0_:
                                case _KEY_F1_:
                                case _KEY_F2_:
                                case _KEY_F3_:
                                case _KEY_F4_:
                                case _KEY_CLEAR_:
				case _KEY_UP_:
                                case _KEY_DOWN_:
                                case _KEY_ENTER_:
				case _KEY_FUNCTION_:
				case _KEY_CANCEL_:
                                        inEVENT_Responder(uszKey);
                                        break;
                                default:
                                        break;
                        }

                }

	}/* IDLE while迴圈 */

#endif
	return (VS_SUCCESS);
}

/*
Function		: inMENU_New_UI
Date&Time	: 2017/9/18 下午 3:26
Describe		: 
 * 因應 "觸控式面板端末設備交易畫面流程需求要點"，所作的新UI
 * 主要負責在主畫面時事件啟動後的對應。
 * 事件觸發後，會執行 inEVENT_Responder() 進行對應按鍵及流程的判斷
 * 事件流程會在此迴圈內重覆判斷，正常狀況下不會跳出此迴圈。
 * 
*/
int inMENU_New_UI()
{

#if (_MACHINE_TYPE_ == _CASTLE_TYPE_V3C_ || _MACHINE_TYPE_ == _CASTLE_TYPE_V3M_ || _MACHINE_TYPE_ == _CASTLE_TYPE_V3P_ || _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_)
	
#ifdef __UPDATE_TMS_AP__	
	int	inEvent = 0;
	int	inTriggerSecond = 0;	/* 多久檢查一次固定時間觸發事件，時間設得太小會造成觸控延遲 */
	char	szOrgTime[6 + 1];	/* 原時間 */
	char	szLastEventTime[6 + 1];/* 用來避免同一分鐘執行兩次事件 */
	char	szNowTime[6 + 1];
	RTC_NEXSYS	srRTC;
#endif	
	int	inRetVal = VS_ERROR;
	char	szCTLSEnable[2 + 1]; /* 觸控是否打開 */

	unsigned char	uszKey = 0;
	unsigned short	usLen = 0;
	
	/* 有觸控才要加 */
#ifdef _TOUCH_CAPBILITY_
	int	inFunc = 0;	 /* 接收觸控回傳，進哪一個畫面 */
#endif
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	
	while (1)
	{

#ifdef _FUBON_MAIN_HOST_	
		/* 初始 KioskFlag 的值 2020/3/6 下午 4:28 [SAM] */
		inFunc_SetKisokFlag("N");
#endif

		/* 初始化 */
		ginIdleMSRStatus = VS_FALSE;	/* idle刷卡 */
		ginIdleICCStatus = VS_FALSE;	/* idle插晶片卡 */
		ginMenuKeyIn = VS_FALSE;	/* idle Keyin */
                
                /* 20230421 Miyano IDLE，EDC狀態設為'I' */
                memset(gsrECR_GASStation_tb.szEDCStatus, 0x00, sizeof(gsrECR_GASStation_tb.szEDCStatus));
                memcpy(gsrECR_GASStation_tb.szEDCStatus, "I", 1);

		/* 顯示IDLE畫面控制，一直Display圖片會造成觸控判斷延遲 */
		if (ginIdleDispFlag == VS_TRUE)
		{
            
#if (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_)
			inEVENT_DSIP_DecideDisplayIdleImageFullScreen();
#else
			inEVENT_DSIP_DecideDisplayIdleImage();
#endif
			/* 已顯示Idle圖，關閉flag */
			ginIdleDispFlag  = VS_FALSE;
		}
		
#ifdef _TOUCH_CAPBILITY_
		/* IDLE觸控畫面 */
		inFunc = inDISP_DisplayIdleMessage_NewUI(); /* 回傳IDLE畫面的按鈕選擇 */

		if (inFunc == _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_)
		{
			inEVENT_Responder(inFunc);
		}
#endif
		/* 感應功能沒開或感應功能初始化失敗時才啟動插卡、刷卡功能  [SAM] 2022/7/21 下午 4:30 */
		if (memcmp(&szCTLSEnable[0], "Y", 1) != 0 || guszCTLSInitiOK != VS_TRUE)
		{
			/* 未開感應 */
			/* 偵測磁條刷卡 */
			inRetVal = inCARD_MSREvent();

			if (inRetVal == VS_SUCCESS)
			{
				inEVENT_Responder(_SWIPE_EVENT_);
			}
			else if (inRetVal == VS_SWIPE_ERROR)
			{
				inDISP_Msg_BMP(_ERR_READ_CARD_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
				/* 刷卡錯誤會蓋idle圖 */
				ginIdleDispFlag = VS_TRUE;
			}

			/* 偵測晶片插卡 */
			inRetVal = inEMV_ICCEvent();
			if (inRetVal == VS_SUCCESS)
			{
				inEVENT_Responder(_EMV_DO_EVENT_);
			}

		}

		/* 偵測ECR收到資料 */
		if (inECR_Receive_Check(&usLen) == VS_SUCCESS)
            	{
			inEVENT_Responder(_ECR_EVENT_);
		}

		/* 顯示刷卡機第一行狀態 */
		inFunc_Display_All_Status_By_Machine_Type();

		if (ginMachineType == _CASTLE_TYPE_V3M_)
		{
			if (inPWM_IS_PWM_Enable() == VS_SUCCESS)
			{
				/* 如果超過idle時間，進入睡眠模式 */
				if (inTimerGet(_TIMER_POWER_MANAGEMENT_) == VS_SUCCESS)
				{
					inEVENT_Responder(_POWER_MANAGEMENT_EVENT_);
				}
			}
		}
                
                /* 20230317 Miyano add for V3P當外接設備 Start */
                /* 偵測多接設備收到資料 */
                if(ginMachineType == _CASTLE_TYPE_V3P_ ||
                   ginMachineType == _CASTLE_TYPE_V3C_)
                {
                    if (inMultiFunc_First_Receive_Check() == VS_SUCCESS)
                    {
                            if (ginDisplayDebug == VS_TRUE)
                            {
                                    char	szDebugMsg[100 + 1];

                                    memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                    sprintf(szDebugMsg, "Receive Event");
                                    inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
                            }

                            {
                                    inFunc_CalculateRunTimeGlobal_Start();
                            }

                            /*若有開工程師debug，只建議直接按0進入 */
                            if (ginEngineerDebug == VS_TRUE)
                            {

                            }
                            else
                            {
                                    inEVENT_Responder(_MULTIFUNC_SLAVE_EVENT_);
                            }
                    }
                }
                /* 20230308 Miyano add for V3C當外接設備 End */
                
		/* 偵測按鍵觸發 */
		uszKey = 0x00;
		uszKey = uszKBD_Key();
		if (uszKey != 0x00)
		{
			switch (uszKey)
			{
				case _KEY_1_:
				case _KEY_2_:
				case _KEY_3_:
				case _KEY_4_:
				case _KEY_5_:
				case _KEY_6_:
				case _KEY_7_:
				case _KEY_8_:
				case _KEY_9_:
				case _KEY_0_:
				case _KEY_F1_:
				case _KEY_F2_:
				case _KEY_F3_:
				case _KEY_F4_:
				case _KEY_CLEAR_:
				case _KEY_DOT_:
				case _KEY_ENTER_:
				case _KEY_FUNCTION_:
				case _KEY_CANCEL_:
					inEVENT_Responder(uszKey);
					break;
				default:
					break;
			}
		}

/* 回到Idle立刻上傳ESC */
/*  因為富邦不需要在idle上傳ESC，所以拿掉 20190108 [SAM]  */	
#if 0
		if (ginESC_Idle_flag == _ESC_IDLE_UPLOAD_ )
		{
			/* Idle上傳ESC */
			inRetVal = inEVENT_Responder(_ESC_IDLE_UPLOAD_EVENT_);
		}
#endif
	}
#endif
	return (VS_SUCCESS);
}

/*
Function        :inMENU_Load_Key_UI
Date&Time   :2017/11/14 下午 3:59
Describe        :
*/
int inMENU_Load_Key_UI()
{
	unsigned char	uszKey = 0;
	unsigned short	usLen = 0;

	while (1)
	{
		/* 顯示IDLE畫面控制，一直Display圖片會造成觸控判斷延遲 */
                if (ginIdleDispFlag == VS_TRUE)
                {
			inDISP_ClearAll();
			inDISP_ChineseFont("1.Load Master Key", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_CENTER_);
			/* 已顯示Idle圖，關閉flag */
                        ginIdleDispFlag  = VS_FALSE;
                }

		/* 偵測ECR收到資料 */
		if (inECR_Receive_Check(&usLen) == VS_SUCCESS)
            	{
			inEVENT_Responder(_ECR_EVENT_);
		}

		/* 偵測按鍵觸發 */
		uszKey = 0x00;
		uszKey = uszKBD_Key();
                if (uszKey != 0x00)
                {
                        switch (uszKey)
                        {
                                case _KEY_1_:
                                case _KEY_2_:
                                case _KEY_3_:
                                case _KEY_4_:
                                case _KEY_5_:
                                case _KEY_6_:
                                case _KEY_7_:
                                case _KEY_8_:
                                case _KEY_9_:
                                case _KEY_0_:
                                case _KEY_F1_:
                                case _KEY_F2_:
                                case _KEY_F3_:
                                case _KEY_F4_:
                                case _KEY_CLEAR_:
                                case _KEY_DOT_:
                                case _KEY_ENTER_:
				case _KEY_FUNCTION_:
				case _KEY_CANCEL_:
                                        inEVENT_Responder(uszKey);
                                        break;
                                default:
                                        break;
                        }

                }

	}

	return (VS_SUCCESS);
}

/*
Function	:inMENU_000_MenuFlow_V3UL
Date&Time	:2017/7/11 上午 10:23
Describe	:選擇功能Menu
*/
int inMENU_000_MenuFlow_V3UL(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		switch (srEventMenuItem->inEventCode)
		{
			case _IdleTouch_KEY_1_ :
			case _IdleTouch_KEY_2_:
			case _IdleTouch_KEY_3_:
				break;
			case _KEY_0_:
				/* 可模擬感應命令 */
				if (ginEngineerDebug == VS_TRUE)
				{
					inEVENT_Responder(_MULTIFUNC_SLAVE_EVENT_);
				}
				break;
			case _KEY_1_:
			case _KEY_2_:
			case _KEY_3_:
			case _KEY_4_:
			case _KEY_5_:
			case _KEY_6_:
			case _KEY_7_:
			case _KEY_8_:
			case _KEY_9_:
				break;
			case _KEY_ENTER_:
				break;
			case _KEY_FUNCTION_:
			case _KEY_F1_:
				srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
				srEventMenuItem->inCode = FALSE;
				/* 輸入管理號碼 */
				if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
				}
				else
				{
					inRetVal = inFunc_Edit_LOGONum();
				}
				break;
				break;
			case _KEY_F2_:
				srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
				srEventMenuItem->inCode = FALSE;
				/* 輸入管理號碼 */
				if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
				}
				else
				{
					inRetVal = inMENU04_NewUiFunctionMenu(srEventMenuItem);
				}
			case _KEY_F3_:
			case _KEY_F4_:
				break;
			case _KEY_CANCEL_:
				if (ginEngineerDebug == VS_TRUE)
				{
					inFunc_Exit_AP();
				}
				break;
			case _KEY_CLEAR_:
				inFunc_ShellCommand_System("ls ./fs_data/LOGO");
				inFunc_ShellCommand_System("ls ./fs_data");
				inFunc_ShellCommand_System("ls ./ICERData/ICERLog");
				inFunc_ShellCommand_System("ls ./");
				break;
			case _KEY_DOT_:
			case _SWIPE_EVENT_ :
			case _EMV_DO_EVENT_ :
			case _ECR_EVENT_ :
				break;
			case _MULTIFUNC_SLAVE_EVENT_:
				inRetVal = inMENU01_MultiFunctionSlave(srEventMenuItem);
				break;
			case _ESC_IDLE_UPLOAD_EVENT_ :
			case _DCC_SCHEDULE_EVENT_ :
			case _TMS_DCC_SCHEDULE_EVENT_ :
			case _TMS_SCHEDULE_INQUIRE_EVENT_ :
			case _TMS_SCHEDULE_DOWNLOAD_EVENT_ :
			case _TMS_PROCESS_EFFECTIVE_EVENT_ :
				break;
			case _BOOTING_EVENT_:
				inRetVal = inMENU01_EdcBooting(srEventMenuItem);
				break;
			default:
				break;
		}
	}

        return (inRetVal);
}

/*
Function	:inMENU_000_MenuFlow_MP200
Date&Time	:2017/10/2 下午 3:52
Describe	:選擇功能Menu
*/
int inMENU_000_MenuFlow_MP200(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

        switch (srEventMenuItem->inEventCode)
	{
		case _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_ :
			inRetVal = inMENU01_FunctionNewUi(srEventMenuItem);
			break;
//		case _KEY_0_:
//			break;
		case _KEY_1_:
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			inRetVal = inMENU01_MenuKeyInAndGetAmount(srEventMenuItem);
			break;
		case _KEY_ENTER_:
			inRetVal = inMENU01_FunctionNewUi(srEventMenuItem);
			break;
		case _KEY_F1_:
		case _KEY_F2_:
			break;
		case _KEY_F3_:
			break;
		case _KEY_F4_:
			break;
		case _KEY_CANCEL_:
			if (ginEngineerDebug == VS_TRUE)
			{
				srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
				srEventMenuItem->inCode = FALSE;
				/* 輸入管理號碼 */
				if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
				}
				else
				{
					exit(0);
				}
			}
			break;
		case _KEY_UP_:
			if (ginEngineerDebug == VS_TRUE)
			{
				inRetVal = inMENU01_ECR(srEventMenuItem);
			}
			break;
		case _KEY_DOWN_:
			srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
			srEventMenuItem->inCode = FALSE;
			/* 輸入管理號碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
			}
			else
			{
				inRetVal = inWiFi_Test_Menu();
			}
			break;
		case _KEY_CLEAR_:
			inFunc_ls("", "./fs_data/LOGO/");
			inFunc_ls("", "./fs_data/");
			inFunc_ls("", "./ICERData/");
			inFunc_ls("", "./ICERData/ICERLog/");
			inFunc_ls("", "./");
			break;
		case _SWIPE_EVENT_ :
			inRetVal = inMENU01_Swipe(srEventMenuItem);
			break;
		case _EMV_DO_EVENT_ :
			inRetVal = inMENU01_ICC(srEventMenuItem);
			break;
		case _ECR_EVENT_ :
			inRetVal = inMENU01_ECR(srEventMenuItem);
			break;
		case _MULTIFUNC_SLAVE_EVENT_:
			inRetVal = inMENU01_MultiFunctionSlave(srEventMenuItem);
			break;
		case _ESC_IDLE_UPLOAD_EVENT_ :
			inRetVal = inMENU01_EscIdleUpload(srEventMenuItem);
			break;
		case _DCC_SCHEDULE_EVENT_ :
			inRetVal = inMENU01_DccSchedule(srEventMenuItem);
			break;
		case _TMS_DCC_SCHEDULE_EVENT_ :
			inRetVal = inMENU01_TmsDccSchedule(srEventMenuItem);
			break;
		case _TMS_SCHEDULE_INQUIRE_EVENT_ :
			inRetVal = inMENU01_TmsSchduleInquire(srEventMenuItem);
			break;
		case _TMS_SCHEDULE_DOWNLOAD_EVENT_ :
			inRetVal = inMENU01_TmsSchduleDownload(srEventMenuItem);
			break;
		case _TMS_PROCESS_EFFECTIVE_EVENT_ :
			inRetVal = inMENU01_TmsProcessEffective(srEventMenuItem);
			break;
		case _BOOTING_EVENT_:
			inRetVal = inMENU01_EdcBooting(srEventMenuItem);
			break;
		case _POWER_MANAGEMENT_EVENT_:
			inRetVal = inMENU01_PowerManagement(srEventMenuItem);
			break;
		default:
			break;
	}

        return (inRetVal);
}

/*
Function		: inMENU_000_MenuFlow_NEWUI
Date&Time	: 2017/10/16 下午 4:43
Describe		: 目的對應在主畫面預計要執行的功能選項，選擇後顯示或執行對的的功能Menu
*/
int inMENU_000_MenuFlow_NEWUI(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szKey = 0x00;
	unsigned char	uszRejudgeBit = VS_FALSE;
	do 
	{
		uszRejudgeBit = VS_FALSE;
		switch (srEventMenuItem->inEventCode)
		{
			case _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_ :
				inRetVal = inMENU01_FunctionNewUi(srEventMenuItem);
				break;
			case _KEY_0_:
				/* 壓住0後3秒內按clear */
				inDISP_Timer_Start(_TIMER_NEXSYS_4_, 3);
				do
				{
					/* 這裡使用uszKBD_Key而不用uszKBD_Key_In是為了不留在Buffer中 */
					szKey = uszKBD_Key();
				}while (szKey == 0 && inTimerGet(_TIMER_NEXSYS_4_) != VS_SUCCESS);

				/* 不是按clear，不能進隱藏選單 */
				if (szKey != _KEY_CLEAR_)
				{
					uszRejudgeBit = VS_TRUE;
					srEventMenuItem->inEventCode = szKey;
					ginEventCode = szKey;
				}
				else
				{
					inFlushKBDBuffer();

					srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
					srEventMenuItem->inCode = FALSE;
					/* 輸入管理號碼 */
					inDISP_ClearAll();
					if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
					{

					}
					else
					{
						inRetVal = inMENU04_NewUiFunctionMenu(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{

						}
						else
						{

						}
					}
				}
				break;
			case _KEY_1_:
			case _KEY_2_:
			case _KEY_3_:
			case _KEY_4_:
			case _KEY_5_:
			case _KEY_6_:
			case _KEY_7_:
			case _KEY_8_:
			case _KEY_9_:
				inRetVal = inMENU01_MenuKeyInAndGetAmount(srEventMenuItem);
				break;
			case _KEY_ENTER_:
				inRetVal = inMENU01_FunctionNewUi(srEventMenuItem);
				break;
			case _KEY_FUNCTION_:
				break;
			case _KEY_F1_:
			case _KEY_F2_:
				break;
			case _KEY_F3_:
				break;
			case _KEY_F4_:
				break;
			case _KEY_CANCEL_:
				if (ginEngineerDebug == VS_TRUE)
				{
					inFunc_Reboot();
				}
				break;
			case _KEY_CLEAR_:
				inFunc_ls("-R", "./");
				break;
			case _KEY_DOT_:
				if (ginEngineerDebug == VS_TRUE)
				{

				}
				break;
			case _SWIPE_EVENT_ :
				inRetVal = inMENU01_Swipe(srEventMenuItem);
				break;
			case _EMV_DO_EVENT_ :
				inRetVal = inMENU01_ICC(srEventMenuItem);
				break;
			case _ECR_EVENT_ :
				inRetVal = inMENU01_ECR(srEventMenuItem);
				break;
			case _MULTIFUNC_SLAVE_EVENT_:
				inRetVal = inMENU01_MultiFunctionSlave(srEventMenuItem);
				break;
			case _ESC_IDLE_UPLOAD_EVENT_ :
				inRetVal = inMENU01_EscIdleUpload(srEventMenuItem);
				break;
			case _DCC_SCHEDULE_EVENT_ :
				inRetVal = inMENU01_DccSchedule(srEventMenuItem);
				break;
			case _TMS_DCC_SCHEDULE_EVENT_ :
				inRetVal = inMENU01_TmsDccSchedule(srEventMenuItem);
				break;
			case _TMS_SCHEDULE_INQUIRE_EVENT_ :
				inRetVal = inMENU01_TmsSchduleInquire(srEventMenuItem);
				break;
			case _TMS_SCHEDULE_DOWNLOAD_EVENT_ :
				inRetVal = inMENU01_TmsSchduleDownload(srEventMenuItem);
				break;
			case _TMS_PROCESS_EFFECTIVE_EVENT_ :
				inRetVal = inMENU01_TmsProcessEffective(srEventMenuItem);
				break;
			case _BOOTING_EVENT_:
				inRetVal = inMENU01_EdcBooting(srEventMenuItem);
				break;
			case _POWER_MANAGEMENT_EVENT_:
				inRetVal = inMENU01_PowerManagement(srEventMenuItem);
				break;
			default:
				break;
		}
	}while (uszRejudgeBit == VS_TRUE);
		
        return (inRetVal);
}

/*
Function	:inMENU_000_MenuFlow_LoadKeyUI
Date&Time	:2017/11/14 下午 4:07
Describe	:選擇功能Menu
*/
int inMENU_000_MenuFlow_LoadKeyUI(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	switch (srEventMenuItem->inEventCode)
	{
		case _KEY_0_:
			break;
		case _KEY_1_:
			inRetVal = inMENU_ECR_OPERATION_LOAD_KEY_FROM_520(srEventMenuItem);
			break;
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			break;
		case _KEY_ENTER_:
//			inRetVal = inMENU_FUNCTION(srEventMenuItem);
			break;
		case _KEY_FUNCTION_:
			break;
		case _KEY_F1_:
		case _KEY_F2_:
			break;
		case _KEY_F3_:
			break;
		case _KEY_F4_:
			break;
		case _KEY_CANCEL_:
			exit(0);
			break;
		case _KEY_CLEAR_:
			break;
		case _KEY_DOT_:
			break;
		case _SWIPE_EVENT_ :
			inRetVal = inMENU01_Swipe(srEventMenuItem);
			break;
		case _EMV_DO_EVENT_ :
			inRetVal = inMENU01_ICC(srEventMenuItem);
			break;
		case _ECR_EVENT_ :
			inRetVal = inMENU01_ECR(srEventMenuItem);
			break;
		case _MULTIFUNC_SLAVE_EVENT_:
			inRetVal = inMENU01_MultiFunctionSlave(srEventMenuItem);
			break;
		case _ESC_IDLE_UPLOAD_EVENT_ :
			inRetVal = inMENU01_EscIdleUpload(srEventMenuItem);
			break;
		case _DCC_SCHEDULE_EVENT_ :
			inRetVal = inMENU01_DccSchedule(srEventMenuItem);
			break;
		case _TMS_DCC_SCHEDULE_EVENT_ :
			inRetVal = inMENU01_TmsDccSchedule(srEventMenuItem);
			break;
		case _TMS_SCHEDULE_INQUIRE_EVENT_ :
			inRetVal = inMENU01_TmsSchduleInquire(srEventMenuItem);
			break;
		case _TMS_SCHEDULE_DOWNLOAD_EVENT_ :
			inRetVal = inMENU01_TmsSchduleDownload(srEventMenuItem);
			break;
		case _TMS_PROCESS_EFFECTIVE_EVENT_ :
			inRetVal = inMENU01_TmsProcessEffective(srEventMenuItem);
			break;
		case _BOOTING_EVENT_:
			inRetVal = inMENU01_EdcBooting(srEventMenuItem);
			break;
		default:
			break;
	}

        return (inRetVal);
}

/*
Function	:inMENU_SELECT_KEY_FUNC
Date&Time	:2015/10/26 下午 2:48
Describe	:功能鍵
*/
int inMENU_SELECT_KEY_FUNC(EventMenuItem *srEventMenuItem)
{
        int     inRetVal = VS_SUCCESS;
        char    szKey;
        VS_BOOL fSelectPage = VS_TRUE;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_FUNCTION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);

        do
        {
                szKey = uszKBD_GetKey(180);

                if (szKey == _KEY_1_)
                {
                        fSelectPage = VS_FALSE;
                        /* ReaderInit */
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_FUNCTION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);
                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;
                        srEventMenuItem->inRunOperationID = _OPERATION_FUN1_READERINIT_;
                        srEventMenuItem->inRunTRTID = FALSE;
                        /* 輸入管理號碼 */
                        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                return (VS_ERROR);
                }
                else if (szKey == _KEY_3_)
                {
                        fSelectPage = VS_FALSE;
                        /* 通訊時間設定 */
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_FUNC3_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);
                        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
                        srEventMenuItem->inCode = FALSE;
                        /* 輸入管理號碼 */
                        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                return (VS_ERROR);

                        /* 功能3選單 */
                        inMENU_FUN3_COMM_TIME_SET(srEventMenuItem);
                }
                else if (szKey == _KEY_4_)
                {
                        fSelectPage = VS_FALSE;
                        /* Signpad列印測試 */
                        inDISP_ClearAll();
                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;
                        srEventMenuItem->inRunOperationID = _OPERATION_FUN4_PRINT_;
                        srEventMenuItem->inRunTRTID = FALSE;
                }
                else if (szKey == _KEY_5_)
                {
                        fSelectPage = VS_FALSE;
                        /* TMS參數下載 */
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_FUNC5_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);
                        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
                        srEventMenuItem->inCode = FALSE;
                        /* 輸入管理號碼 */
                        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                return (VS_ERROR);

                        /* 功能5選單 */
                        inMENU_FUN5_TMS(srEventMenuItem);
                }
                else if (szKey == _KEY_6_)
                {
                        fSelectPage = VS_FALSE;
                        /* 商店可用的參數下載 */
                        inDISP_ClearAll();
                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;
                        /* 功能6選單 */
                        inMENU_FUN6_SELECT(srEventMenuItem);

                }
                else if (szKey == _KEY_7_)
                {
                        fSelectPage = VS_FALSE;
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_FUNC7_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 功能七 */
                        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
                        srEventMenuItem->inCode = FALSE;
                        /* 輸入管理號碼 */
                        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                return (VS_ERROR);

                        /* 列印參數表 */
                        srEventMenuItem->inRunOperationID = _OPERATION_FUN7_PRINT_;
                        srEventMenuItem->inRunTRTID = FALSE;
                }
		else if (szKey == _KEY_8_)
                {
//                        fSelectPage = VS_FALSE;
//                        inDISP_ClearAll();
//                        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
//                        srEventMenuItem->inCode = FALSE;
//                        /* 輸入管理號碼 */
//                        if (inCheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
//                                return (VS_ERROR);
//
//                        /* 列印參數表 */
//                        srEventMenuItem->inRunOperationID = _OPERATION_FUN8_PRINT_;
//                        srEventMenuItem->inRunTRTID = FALSE;
                }
                else if (szKey == _KEY_9_)
                {
                        /* AP更新測試 */
                        char szTemplate[32 + 1];
                        inDISP_ClearAll();
                        inDISP_ChineseFont("AP更新中", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
                        system("test -d /home/ap/pub/install || mkdir -p /home/ap/pub/install/");
                        system("cp -vfr * /home/ap/pub/install/.");
                        system("ls /home/ap/pub/install/");
                        inRetVal = CTOS_UpdateFromMMCIEx((unsigned char *)"/home/ap/pub/install/update.mmci", &callbackFun);
                        CTOS_UpdateGetResult();
                        sprintf(szTemplate, "inRetVal = 0x%04X", inRetVal);
                        inDISP_LogPrintf(szTemplate);
                        CTOS_SystemReset();
                }
                else if (szKey == _KEY_0_)
                {
                        /* xml測試 */
//                        WenswriteXML();
                        vdEMVXML_write_test();
                        inDISP_ChineseFont("XML測試", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
//                        CTOS_LCDTPrintXY(1, 1, "Any Key to Cont.");
//                        DumpAllXmlDoc("test.xml");
//                        DumpAllXmlDoc("emv_config.xml");
                        inFILE_Test();

                }
                else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
                {
                        inRetVal = VS_ERROR;
                        fSelectPage = VS_FALSE;
                }
                else
                        continue;
                break;

        }while (fSelectPage == VS_TRUE);

        return (inRetVal);
}

int inMENU_SELECT_KEY_FUNC_BAK(EventMenuItem *srEventMenuItem)
{
        int     inRetVal = VS_SUCCESS;
//        char    szKey;
//        VS_BOOL fSelectPage = VS_TRUE;

//        inDISP_ClearAll();
//        inDISP_PutGraphic(_MENU_PRT_REPORT_, 0,  0);
//
//        do
//        {
//                szKey = szKBD_GetKey(180);
//
//                if (szKey == _KEY_1_)
//                {
//                        fSelectPage = VS_FALSE;
//                        inDISP_ClearAll();
//                        /* 第三層顯示 ＜總額報表＞ */
//                        inDISP_PutGraphic(_MENU_TOTAIL_TITLE_, 0,  0);
//                        /* 輸入密碼的層級 */
//                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
//                        srEventMenuItem->inCode = FALSE;
//
//                        /* 第一層輸入密碼 */
//                        if (inCheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
//                                return (VS_ERROR);
//
//                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
//                        srEventMenuItem->inCode = FALSE;
//                        srEventMenuItem->inRunOperationID = _OPERATION_DELETE_BATCH_;
//                        srEventMenuItem->inRunTRTID = FALSE;
//                }
//                else if (szKey == _KEY_2_)
//                {
//                        fSelectPage = VS_FALSE;
//                        inDISP_ClearAll();
//                        /* 第三層顯示 ＜明細報表＞ */
//                        inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0,  0);
//                        /* 輸入密碼的層級 */
//                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
//                        srEventMenuItem->inCode = FALSE;
//
//                        /* 第一層輸入密碼 */
//                        if (inCheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
//                                return (VS_ERROR);
//
//                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
//                        srEventMenuItem->inCode = FALSE;
//                        srEventMenuItem->inRunOperationID = _OPERATION_DETAIL_REPORT_;
//                        srEventMenuItem->inRunTRTID = FALSE;
//                }
//                else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
//                {
//                        inRetVal = VS_ERROR;
//                        fSelectPage = VS_FALSE;
//                }
//                else
//                        continue;
//
//                break;
//        } while (fSelectPage == VS_TRUE);

        return (inRetVal);
}

/*
Function	:inMENU_SELECT_KEY_F3
Date&Time	:2015/8/20 下午 2:48
Describe	:列印報表
*/
int inMENU_SELECT_KEY_F3(EventMenuItem *srEventMenuItem)
{
        int	inRetVal = VS_SUCCESS;
        unsigned char   uszKey;
	VS_BOOL	fSelectPage = VS_TRUE;

	inDISP_ClearAll();
 //       inDISP_PutGraphic(_MENU_PRT_REPORT_, 0,  0);

        do
        {
                uszKey = uszKBD_GetKey(180);

                if (uszKey == _KEY_1_)
                {
                        fSelectPage = VS_FALSE;
                        inDISP_ClearAll();
                        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TOTAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 總額報表 */
                        /* 輸入密碼的層級 */
                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;

                        /* 第一層輸入密碼 */
                        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                return (VS_ERROR);

                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;
                        srEventMenuItem->inRunOperationID = _OPERATION_TOTAL_REPORT_;
                        srEventMenuItem->inRunTRTID = FALSE;
                }
                else if (uszKey == _KEY_2_)
                {
                        fSelectPage = VS_FALSE;
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 明細報表 */
                        /* 輸入密碼的層級 */
                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;

                        /* 第一層輸入密碼 */
                        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                return (VS_ERROR);

                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;
                        srEventMenuItem->inRunOperationID = _OPERATION_DETAIL_REPORT_;
                        srEventMenuItem->inRunTRTID = FALSE;
                }
                else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
                {
                        inRetVal = VS_ERROR;
                        fSelectPage = VS_FALSE;
                }
                else
                        continue;

                break;
        } while (fSelectPage == VS_TRUE);

	return (inRetVal);
}



/*
Function	:inMENU_FUN3_TMS
Date&Time	:2016/1/28 下午 8:14
Describe	:功能3選單 1.通訊設定 2.時間設定 3.版本查詢
*/
int inMENU_FUN3_COMM_TIME_SET(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
        int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_8X16_OPT_;
	char	szKey = 0x00;

        inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_FUNC3_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);

	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	srEventMenuItem->inPasswordLevel =  _ACCESS_FREELY_;

	/* 功能三選單 */
	inDISP_PutGraphic(_MENU_FUNC3_OPTION_, 0,  _COORDINATE_Y_LINE_8_4_);

        inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
        while (1)
        {
                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

                if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
                {
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <通訊設定> */
			srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
			srEventMenuItem->inCode = FALSE;
                        /* 通訊設定 */
                        inRetVal = inCOMM_Fun3_SetCommWay();
                        break;
                }
                else if (szKey == _KEY_2_			||
			 inChoice == _OPTTouch8X16_LINE_6_)
                {
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SET_DATE_TIME_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜時間設定＞ */
			srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
			srEventMenuItem->inCode = FALSE;
                        /* 時間設定 */
                        inRetVal = inFunc_Fun3EditDateTime();
			break;
                }
                else if (szKey == _KEY_3_			||
			 inChoice == _OPTTouch8X16_LINE_7_)
                {
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_AP_VERSION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜版本查詢> */
			srEventMenuItem->inPasswordLevel =  _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;
                        /* 版本查詢 */
                        inRetVal = inFunc_Check_Version_ID();
                        break;
                }
                else if (szKey == _KEY_CANCEL_)
                {
                        inRetVal = VS_USER_CANCEL;
			break;
                }
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
        }
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}

/*
Function	:inMENU_FUN5_TMS
Date&Time	:2016/1/4 下午 3:02
Describe	:功能5 TMS下載選單 1.參數下載 2.至現回報
*/
int inMENU_FUN5_TMS(EventMenuItem *srEventMenuItem)
{
        int	inRetVal = VS_SUCCESS;
        int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_8X16_OPT_;
	char	szKey = 0x00;

	/* 功能五 */
        inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_FUNC5_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	inDISP_PutGraphic(_MENU_FUNC5_OPTION_, 0,  _COORDINATE_Y_LINE_8_4_);
        inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
        while (1)
        {
                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

                if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
                {
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
			/* 輸入密碼的層級 */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;

			/* 第一層輸入密碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}

			srEventMenuItem->inRunOperationID = _OPERATION_FUN5_TMS_DOWNLOAD_;
			srEventMenuItem->inRunTRTID = FALSE;

                        break;
                }
                else if (szKey == _KEY_2_			||
			 inChoice == _OPTTouch8X16_LINE_6_)
                {
                        inDISP_ClearAll();
			/* 第三層顯示 ＜DCC參數下載＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DCC_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC參數下載＞ */
			/* 輸入密碼的層級 */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;

			/* 第一層輸入密碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}

			srEventMenuItem->inRunOperationID = _OPERATION_FUN6_DCC_DOWNLOAD_;
			srEventMenuItem->inRunTRTID = FALSE;

                        break;
                }
		else if (szKey == _KEY_3_			||
			 inChoice == _OPTTouch8X16_LINE_7_)
                {
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TMS_TASK_MENU_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜至現回報＞ */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;
			/* 輸入管理號碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}
			srEventMenuItem->inRunOperationID = _OPERATION_FUN5_TMS_TASK_REPORT_;
			srEventMenuItem->inRunTRTID = FALSE;

                        break;
                }
                else if (szKey == _KEY_CANCEL_)
                {
                        inRetVal = VS_USER_CANCEL;
			break;
                }
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
        }
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}

/*
Function	:inMENU_FUN6_SELECT
Date&Time	:2016/1/4 下午 3:02
Describe	:功能6選單 1.設定管理號碼 2.資訊回報 3.參數下載 4.版本查詢
*/
int inMENU_FUN6_SELECT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_8X16_OPT_;
	char	szKey = 0x00;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_FUNC6_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 2u4*/
	inDISP_PutGraphic(_MENU_FUNC6_OPTION_, 0,  _COORDINATE_Y_LINE_8_4_);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SET_PWD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 設定管理號碼 */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;
			/* 輸入管理號碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
			inRetVal = VS_ERROR;
			break;
			}
			inRetVal = inFunc_EditPWD_Flow();

			break;
		}
		else if (szKey == _KEY_2_			||
			inChoice == _OPTTouch8X16_LINE_6_)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_RETURN_PARAM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜資訊回報＞ */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;
			/* 輸入管理號碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}
			srEventMenuItem->inRunOperationID = _OPERATION_FUN6_TMS_TRACELOG_UP_;
			break;
		}
		else if (szKey == _KEY_3_			||
			inChoice == _OPTTouch8X16_LINE_7_)
		{
			/* 參數下載 */
			inRetVal = inMENU_Download_Parameter(srEventMenuItem);
			break;
		}
		else if (szKey == _KEY_4_			||
			inChoice == _OPTTouch8X16_LINE_8_)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_AP_VERSION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜版本查詢> */
			srEventMenuItem->inPasswordLevel =  _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;
			/* 版本查詢 */
			inRetVal = inFunc_Check_Version_ID();
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}


/*
Function	:inMENU_ECR_OPERATION_LOAD_KEY_FROM_520
Date&Time	:2017/11/15 上午 11:112016/6/23 上午 11:18
Describe	:ECR連線事件
*/
int inMENU_ECR_OPERATION_LOAD_KEY_FROM_520(EventMenuItem *srEventMenuItem)
{
	inDISP_ClearAll();

        /* 第三層顯示 ＜ECR連線＞ */
	inDISP_PutGraphic(_ECR_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);
        /* 這裡不設incode */

        /* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_LOAD_KEY_FROM_520_;
        /* 這裡不設TRT */

        return (VS_SUCCESS);
}


/*
Function        : inMENU_ETICKET_DEDUCT
Date&Time   :  2022/6/13 下午 3:17
Describe        :
 * [新增電票悠遊卡功能] 重新啟用此功能 [SAM] 
*/
int inMENU_ETICKET_DEDUCT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inNCCC_Ticket_Func_Check_Transaction_Deduct(_TICKET_DEDUCT_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_DEDUCT_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
		srEventMenuItem->inRunTRTID = FALSE;
		srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        : inMENU_ETICKET_REFUND
Date&Time   : 2022/6/13 下午 3:18
Describe        :
 * [新增電票悠遊卡功能] 重新啟用此功能 [SAM] 
*/
int inMENU_ETICKET_REFUND(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inNCCC_Ticket_Func_Check_Transaction_Refund(_TICKET_IPASS_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退貨交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_TICKET_REFUND_;
		srEventMenuItem->inRunTRTID = FALSE;
		srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        : inMENU_ETICKET_INQUIRY
Date&Time   : 2022/6/13 下午 3:19
Describe        :
 * [新增電票悠遊卡功能] 重新啟用此功能 [SAM] 
*/
int inMENU_ETICKET_INQUIRY(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inNCCC_Ticket_Func_Check_Transaction_Inquiry(_TICKET_IPASS_INQUIRY_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜餘額查詢＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_INQUIRY_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_TICKET_INQUIRY_;
		srEventMenuItem->inRunTRTID = FALSE;
		srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        : inMENU_ETICKET_TOP_UP
Date&Time   : 2022/6/13 下午 3:20
Describe        :
 * [新增電票悠遊卡功能] 重新啟用此功能 [SAM] 
*/
int inMENU_ETICKET_TOP_UP(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inNCCC_Ticket_Func_Check_Transaction_Top_Up(_TICKET_IPASS_TOP_UP_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_TOP_UP_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_TICKET_TOP_UP_;
		srEventMenuItem->inRunTRTID = FALSE;
		srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        : inMENU_ETICKET_VOID_TOP_UP
Date&Time   : 2022/6/13 下午 3:21
Describe        :
 * [新增電票悠遊卡功能] 重新啟用此功能 [SAM] 
*/
int inMENU_ETICKET_VOID_TOP_UP(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inNCCC_Ticket_Func_Check_Transaction_Void_Top_Up(_TICKET_IPASS_VOID_TOP_UP_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值取消＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_VOID_TOP_UP_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_TICKET_VOID_TOP_UP_;
		srEventMenuItem->inRunTRTID = FALSE;
		srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_AWARD_SWIPE
Date&Time       :2017/10/30 上午 8:57
Describe        :
*/
int inMENU_AWARD_SWIPE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

//	if (inNCCC_Loyalty_CreditCardFlag(_LOYALTY_REDEEM_) != VS_SUCCESS)
	if(1)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜優惠兌換＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _LOYALTY_REDEEM_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
		srEventMenuItem->inRunOperationID = _OPERATION_LOYALTY_REDEEM_CTLS_;
		srEventMenuItem->inRunTRTID = _TRT_LOYALTY_REDEEM_;
	}

	return (inRetVal);
}

/*
Function        :inMENU_AWARD_BARCODE
Date&Time       :2017/10/30 下午 1:19
Describe        :
*/
int inMENU_AWARD_BARCODE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

//	if (inNCCC_Loyalty_BarCodeFlag(_LOYALTY_REDEEM_) != VS_SUCCESS)
	if(1)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜優惠兌換＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _LOYALTY_REDEEM_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
		srEventMenuItem->inRunOperationID = _OPERATION_BARCODE_;
		srEventMenuItem->inRunTRTID = _TRT_LOYALTY_REDEEM_;
	}

	return (inRetVal);
}

/*
Function        :inMENU_AWARD_VOID
Date&Time       :2017/10/30 下午 1:19
Describe        :
*/
int inMENU_AWARD_VOID(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

//	if (inNCCC_Loyalty_VoidRedeemFlag(_VOID_LOYALTY_REDEEM_) != VS_SUCCESS)
	if(1)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_VOID_LOYALTY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜兌換取消＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _VOID_LOYALTY_REDEEM_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
		srEventMenuItem->inRunOperationID = _OPERATION_VOID_LOYALTY_REDEEM_;
		srEventMenuItem->inRunTRTID = _TRT_VOID_LOYALTY_REDEEM_;
	}

	return (inRetVal);
}

/*
Function        :inMENU_AWARD_REFUND
Date&Time       :2017/10/30 下午 1:19
Describe        :
*/
int inMENU_AWARD_REFUND(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

//	if (inNCCC_Loyalty_RefundFlag(_LOYALTY_REDEEM_REFUND_) != VS_SUCCESS)
	if(1)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_VOID_LOYALTY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜兌換取消＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _LOYALTY_REDEEM_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
		srEventMenuItem->inRunOperationID = _OPERATION_LOYALTY_REDEEM_REFUND_;
		srEventMenuItem->inRunTRTID = _TRT_LOYALTY_REDEEM_REFUND_;
	}

	return (inRetVal);
}

/*
Function	:inMENU_DCC_RATE
Date&Time	:2017/2/9 下午 12:01
Describe	:DCC匯率下載
*/
int inMENU_DCC_RATE(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DCC_RATE_DOWNLOAD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC匯率下載＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inRunOperationID = _OPERATION_FUN6_DCC_RATE_DOWNLOAD_;
	srEventMenuItem->inRunTRTID = FALSE;

	inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
	inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);

	uszKey = 0;
	while (1)
	{

		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_0_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}

	}

	return (inRetVal);
}

/*
Function		: inMENU_Download_Parameter
Date&Time	: 2017/5/19 下午 6:34
Describe		: 參數下載
*/
int inMENU_Download_Parameter(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
	{
		inRetVal = VS_ERROR;
		//break;
		return inRetVal;
	}

	srEventMenuItem->inRunOperationID = _OPERATION_TMS_DOWNLOAD_;
	srEventMenuItem->inRunTRTID = FALSE;

	inRetVal = VS_SUCCESS;
	
/* 以下為有DCC下載的範例，保留著參考用 */
#if 0
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char	szKey = 0x00;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_PARAM_DOWNLOAD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 參數下載 */
	inDISP_PutGraphic(_MENU_PARA_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 參數下載 */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* TMS參數下載 */
		if (szKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
			/* 輸入密碼的層級 */
			srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
			srEventMenuItem->inCode = FALSE;

			/* 第一層輸入密碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				//break;
				return inRetVal;
			}

			srEventMenuItem->inRunOperationID = _OPERATION_TMS_DOWNLOAD_;
			srEventMenuItem->inRunTRTID = FALSE;

			inRetVal = VS_SUCCESS;

			break;
		}
		/* DCC參數下載 */
		else if (szKey == _KEY_2_ ||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DCC_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC參數下載＞ */
			/* 輸入密碼的層級 */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;

			/* 第一層輸入密碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}

			srEventMenuItem->inRunOperationID = _OPERATION_FUN6_DCC_DOWNLOAD_;
			srEventMenuItem->inRunTRTID = FALSE;

			inRetVal = VS_SUCCESS;
			break;
		}
		/* DCC匯率下載 */
		else if (szKey == _KEY_3_			||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
		{
			inRetVal = inMENU_DCC_RATE(srEventMenuItem);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
#endif

	return (inRetVal);
}

/*
Function        :inMENU_VOID
Date&Time       :2017/10/26 上午 11:13
Describe        :
*/
int inMENU_VOID(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_VOID_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _VOID_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_VOID_;
		srEventMenuItem->inRunTRTID = _TRT_VOID_;
	}

	return (inRetVal);
}

/*
Function        :inMENU_REFUND
Date&Time       :2017/8/25 下午 6:07
Describe        :
*/
int inMENU_REFUND(EventMenuItem *srEventMenuItem)
{
	int			inRetVal = VS_SUCCESS;
//	int			inChoice = 0;
//	int			inTouchSensorFunc = _Touch_NONE_;
//	char			szKey = 0x00;
//	char			szFunEnable[2 + 1] = {0};
//	MENU_CHECK_TABLE	srMenuChekDisplay[] =
//	{
//		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _TRANS_TYPE_NULL_	, inMENU_Check_HG_Enable		, _ICON_HIGHTLIGHT_1_2_2_HG_REFUND_		},
//		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL					, ""						}
//	};

	if (inCREDIT_CheckTransactionFunction(_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		/* 第三層顯示 ＜退貨交易＞ */
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

		inRetVal = inMENU_CREDIT_REFUND(srEventMenuItem);
		if (inRetVal == VS_USER_CANCEL)
		{
			inRetVal = VS_ERROR;
		}
		else if (inRetVal == VS_FUNC_CLOSE_ERR)
		{
			inRetVal = VS_SUCCESS;
		}
		else
		{
			/* 回傳inRetVal */
		}
	}

	return (inRetVal);
}

/*
Function        :inMENU_CREDIT_REFUND
Date&Time       :2017/8/28 上午 11:09
Describe        :
*/
int inMENU_CREDIT_REFUND(EventMenuItem *srEventMenuItem)
{
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _REFUND_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_2_1_1_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _REDEEM_REFUND_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_2_1_2_REDEEM_REFUND_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_	, _INST_REFUND_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_2_1_3_INST_REFUND_		},
		{_Touch_NONE_			, _TRANS_TYPE_NULL_	, NULL						, ""						}
	};

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_MENU_REFUND_OPTION_2_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 檢查功能開關，並顯示反白的圖 */
	inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* 一般退貨 */
		if (szKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
		{
			inRetVal = inMENU_REFUND_NORMAL(srEventMenuItem);
			if (inRetVal == VS_USER_CANCEL)
			{
				inRetVal = VS_ERROR;
				break;
			}
			else if (inRetVal == VS_FUNC_CLOSE_ERR)
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{
				break;
			}
		}
		/* 紅利退貨 */
		else if (szKey == _KEY_2_			||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
		{
			inRetVal = inMENU_REFUND_REDEEM(srEventMenuItem);
			if (inRetVal == VS_USER_CANCEL)
			{
				inRetVal = VS_ERROR;
				break;
			}
			else if (inRetVal == VS_FUNC_CLOSE_ERR)
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{
				break;
			}
		}
		/* 分期退貨 */
		else if (szKey == _KEY_3_			||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
		{
			inRetVal = inMENU_REFUND_INST(srEventMenuItem);
			if (inRetVal == VS_USER_CANCEL)
			{
				inRetVal = VS_ERROR;
				break;
			}
			else if (inRetVal == VS_FUNC_CLOSE_ERR)
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{
				break;
			}
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}

/*
Function        :inMENU_REFUND_NORMAL
Date&Time       :2017/11/7 下午 5:29
Describe        :
*/
int inMENU_REFUND_NORMAL(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szCTLSEnable[2 + 1];

	if (inCREDIT_CheckTransactionFunction(_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);

		inDISP_ClearAll();
		/* 第三層顯示 ＜退貨交易＞ */
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
		{
			srEventMenuItem->inRunOperationID = _OPERATION_REFUND_CTLS_;
			srEventMenuItem->inRunTRTID = _TRT_REFUND_;
		}
		else
		{
			srEventMenuItem->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
			srEventMenuItem->inRunTRTID = _TRT_REFUND_CTLS_;
		}
	}

	return (inRetVal);
}

/*
Function        :inMENU_REFUND_REDEEM
Date&Time       :2017/11/7 下午 5:29
Describe        :
*/
int inMENU_REFUND_REDEEM(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szCTLSEnable[2 + 1];

	if (inCREDIT_CheckTransactionFunction(_REDEEM_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);

		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _REDEEM_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		/*if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
		{
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("inMENU_REFUND_REDEEM() 1 !!");
			srEventMenuItem->inRunOperationID = _OPERATION_REDEEM_REFUND_CTLS_;
			srEventMenuItem->inRunTRTID = _TRT_REDEEM_REFUND_CTLS_;
		}
		else*/
		{
			if (ginDebug == VS_TRUE)
				inDISP_LogPrintf("inMENU_REFUND_REDEEM() 2 !!");
			srEventMenuItem->inRunOperationID = _OPERATION_REFUND_;
			srEventMenuItem->inRunTRTID = _TRT_REDEEM_REFUND_;
		}
		srEventMenuItem->uszRedeemBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_REFUND_INST
Date&Time       :2017/11/7 下午 5:29
Describe        :
*/
int inMENU_REFUND_INST(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szCTLSEnable[2 + 1];

	if (inCREDIT_CheckTransactionFunction(_INST_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);

		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _INST_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		/*if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
		{
			srEventMenuItem->inRunOperationID = _OPERATION_INST_REFUND_CTLS_;
			srEventMenuItem->inRunTRTID = _TRT_INST_REFUND_CTLS_;
		}
		else*/
		{
			srEventMenuItem->inRunOperationID = _OPERATION_REFUND_;
			srEventMenuItem->inRunTRTID = _TRT_INST_REFUND_;
		}
		srEventMenuItem->uszInstallmentBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_INST_REDEEM
Date&Time       :2017/8/25 下午 6:00
Describe        :
*/
int inMENU_INST_REDEEM(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_8X16_OPT_;
	char	szKey = 0x00;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REDEEM_INST_TITTLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期紅利＞ */
	inDISP_PutGraphic(_MENU_REDEEM_INST_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* 一般紅利 */
		if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
		{
			inRetVal = inMENU_REDEEM(srEventMenuItem);
			break;
		}
		/* 一般分期*/
		else if (szKey == _KEY_2_			||
		         inChoice == _OPTTouch8X16_LINE_6_)
		{
			inRetVal = inMENU_INST(srEventMenuItem);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}

/*
Function        :inMENU_REDEEM
Date&Time       :2017/10/26 上午 11:36
Describe        :
*/
int inMENU_REDEEM(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_REDEEM_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _REDEEM_SALE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_I_R_;
		srEventMenuItem->inRunTRTID = _TRT_REDEEM_SALE_;
		srEventMenuItem->uszRedeemBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_INST
Date&Time       :2017/10/26 上午 11:36
Describe        :
*/
int inMENU_INST(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_INST_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _INST_SALE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_I_R_;
		srEventMenuItem->inRunTRTID = _TRT_INST_SALE_;
		srEventMenuItem->uszInstallmentBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_SETTLE
Date&Time       :2017/8/28 下午 1:45
Describe        :
*/
int inMENU_SETTLE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_8X16_OPT_;
	char	szKey = 0x00;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */
	/* 顯示連動結帳 OR 個別結帳 */
	inDISP_PutGraphic(_MENU_SETTLE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* 連動結帳 */
		if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
		{
			inRetVal = inMENU_SETTLE_AUTOSETTLE(srEventMenuItem);
			break;
		}
		/* 個別結帳*/
		else if (szKey == _KEY_2_			||
			 inChoice == _OPTTouch8X16_LINE_6_)
		{
			inRetVal = inMENU_SETTLE_BY_HOST(srEventMenuItem);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}

/*
Function        :inMENU_SETTLE_AUTOSETTLE
Date&Time       :2017/10/30 下午 2:14
Describe        :
*/
int inMENU_SETTLE_AUTOSETTLE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */

	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
	srEventMenuItem->inCode = _SETTLE_;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
	{
		inRetVal = VS_ERROR;
	}

	srEventMenuItem->inRunOperationID = _OPERATION_SETTLE_;
	srEventMenuItem->inRunTRTID = _TRT_SETTLE_;

	/* 開啟連動結帳flag */
	srEventMenuItem->uszAutoSettleBit = VS_TRUE;

	return (inRetVal);
}

/*
Function        :inMENU_SETTLE_BY_HOST
Date&Time       :2017/10/30 下午 2:15
Describe        :
*/
int inMENU_SETTLE_BY_HOST(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */

	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
	srEventMenuItem->inCode = _SETTLE_;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
	{
		inRetVal = VS_ERROR;
	}

	srEventMenuItem->inRunOperationID = _OPERATION_SETTLE_;
	srEventMenuItem->inRunTRTID = _TRT_SETTLE_;

	return (inRetVal);
}

/*
Function        inMENU_SALEOFFLINE
Date&Time       :2017/10/26 上午 11:15
Describe        :
*/
int inMENU_SALEOFFLINE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_SALE_OFFLINE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _SALE_OFFLINE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_SALE_OFFLINE_;
		srEventMenuItem->inRunTRTID = _TRT_SALE_OFFLINE_;
	}

	return (inRetVal);
}

/*
Function        inMENU_TIP
Date&Time       :2017/10/26 上午 11:15
Describe        :
*/
int inMENU_TIP(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;


	if (inCREDIT_CheckTransactionFunction(_TIP_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TIP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜小費交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _TIP_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_TIP_;
		srEventMenuItem->inRunTRTID = _TRT_TIP_;
	}

	return (inRetVal);
}
/*
Function        : inMENU_PREAUTH
Date&Time   : 2022/4/13 下午 5:51
Describe        : 
 *  主要顯示預先授權畫面，修改為使用 Defined 來區分要使用的方法
 * 
*/

 /* 修改富邦需求，由原本只支援 _PRE_AUTH_  插卡與刷卡，
  * 改為可支援感應及 _PRE_COMP_ 交易 2022/4/13 [SAM] */
int inMENU_PREAUTH(EventMenuItem *srEventMenuItem)
{
        int	inRetVal = VS_SUCCESS;
        int	inChoice = 0;
        int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
        char	szKey = 0x00;
        char	szCTLSEnable[2 + 1];

        if (inCREDIT_CheckTransactionFunction(_PRE_AUTH_) != VS_SUCCESS)
        {
                inRetVal = VS_FUNC_CLOSE_ERR;
        }
        else
        {
                memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
                inGetContactlessEnable(szCTLSEnable);

                inDISP_ClearAll();
                /* 第三層顯示 ＜預先授權＞ */
                inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞<預授權完成> */
                inDISP_PutGraphic(_MENU_PRE_AUTH_OPTION_1_, 0, _COORDINATE_Y_LINE_8_4_);

                inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

                while (1)
                {
                        inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                        szKey = uszKBD_Key();

                        /* Timeout */
                        if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
                        {
                                szKey = _KEY_TIMEOUT_;
                        }

                        /* 預先授權 */
                        if (szKey == _KEY_1_			||
                            inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
                        {
                                inDISP_ClearAll();
                                inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                                inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */
                                /* 輸入密碼的層級 */
                                srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
                                srEventMenuItem->inCode = _PRE_AUTH_;

                                /* 第一層輸入密碼 */
                                if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                {
                                        inRetVal = VS_ERROR;
                                        break;
                                }

                                if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
                                {
                                        srEventMenuItem->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
                                        srEventMenuItem->inRunTRTID = _TRT_PRE_AUTH_CTLS_;
                                }
                                else
                                {
                                        srEventMenuItem->inRunOperationID = _OPERATION_PRE_AUTH_;
                                        srEventMenuItem->inRunTRTID = _TRT_PRE_AUTH_;
                                }

                                inRetVal = VS_SUCCESS;
                                break;
                        }
                        /* 授權完成 */
                        else if (szKey == _KEY_2_			||
                             inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
                        {
                                inDISP_ClearAll();
                                inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
                                inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */
                                /* 輸入密碼的層級 */
                                srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
                                srEventMenuItem->inCode = _PRE_COMP_;

                                /* 第一層輸入密碼 */
                                if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                {
                                    inRetVal = VS_ERROR;
                                    break;
                                }
                                
                                if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
                                {
                                        srEventMenuItem->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
                                        srEventMenuItem->inRunTRTID = _TRT_PRE_COMP_CTLS_;
                                }else{
                                        srEventMenuItem->inRunOperationID = _OPERATION_PRE_COMP_;
                                        srEventMenuItem->inRunTRTID = _TRT_PRE_COMP_;
                                }

                                inRetVal = VS_SUCCESS;
                                break;
                        }
                        else if (szKey == _KEY_CANCEL_)
                        {
                                inRetVal = VS_USER_CANCEL;
                                break;
                        }
                        else if (szKey == _KEY_TIMEOUT_)
                        {
                                inRetVal = VS_TIMEOUT;
                                break;
                        }

                }
                /* 清空Touch資料 */
                inDisTouch_Flush_TouchFile();
        }
        
        return (inRetVal);
}	

/*
Function        :inMENU_MAILORDER
Date&Time       :2017/10/26 上午 11:21
Describe        :
*/
int inMENU_MAILORDER(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_MAIL_ORDER_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_MAIL_ORDER_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜郵購交易＞ */

		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _MAIL_ORDER_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_MAIL_ORDER_;
		srEventMenuItem->inRunTRTID = _TRT_MAIL_ORDER_;
		srEventMenuItem->uszMailOrderBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_SALE
Date&Time       :2017/10/26 上午 11:24
Describe        :
*/
int inMENU_SALE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	/* 第三層顯示 ＜一般交易＞ */
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
	srEventMenuItem->inCode = _SALE_;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inRunOperationID = _OPERATION_SALE_CTLS_;
	srEventMenuItem->inRunTRTID = _TRT_SALE_;

	return (inRetVal);
}

/*
Function        :inMENU_ADJUST
Date&Time       :2017/8/23 下午 5:08
Describe        :
*/
int inMENU_ADJUST(EventMenuItem *srEventMenuItem)
{
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szKey= 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _REDEEM_ADJUST_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_8_1_REDEEM_ADJUST_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _INST_ADJUST_		, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_1_8_2_INST_ADJUST_		},
		{_Touch_NONE_			, _TRANS_TYPE_NULL_	, NULL						, ""						}
	};

	if (inCREDIT_CheckTransactionFunction(_INST_ADJUST_) != VS_SUCCESS	&&
	    inCREDIT_CheckTransactionFunction(_REDEEM_ADJUST_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜後台調帳＞ */
		inDISP_PutGraphic(_MENU_ADJUST_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			/* 紅利調帳 */
			if (szKey == _KEY_1_			||
			    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
			{
				inRetVal = inMENU_REDEEM_ADJUST(srEventMenuItem);
				if (inRetVal == VS_USER_CANCEL)
				{
					inRetVal = VS_ERROR;
					break;
				}
				else if (inRetVal == VS_FUNC_CLOSE_ERR)
				{
					inRetVal = VS_SUCCESS;
				}
				else
				{
					break;
				}
			}
			/* 分期調帳 */
			else if (szKey == _KEY_2_			||
				 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
			{
				inRetVal = inMENU_INST_ADJUST(srEventMenuItem);
				if (inRetVal == VS_USER_CANCEL)
				{
					inRetVal = VS_ERROR;
					break;
				}
				else if (inRetVal == VS_FUNC_CLOSE_ERR)
				{
					inRetVal = VS_SUCCESS;
				}
				else
				{
					break;
				}
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}

		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}

	return (inRetVal);
}

/*
Function        :inMENU_SALE_ADJUST
Date&Time       :2017/11/8 上午 10:08
Describe        :
*/
int inMENU_SALE_ADJUST(EventMenuItem *srEventMenuItem)
{
int	inRetVal = VS_SUCCESS;

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inMENU_SALE_ADJUST 1");

	if (inCREDIT_CheckTransactionFunction(_ADJUST_) != VS_SUCCESS)
	{

		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inMENU_SALE_ADJUST 2");
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{

		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inMENU_SALE_ADJUST 3");
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_ADJUST_1_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜小費交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _ADJUST_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		if (ginDebug == VS_TRUE)
			inDISP_LogPrintf("inMENU_SALE_ADJUST 4");

		srEventMenuItem->inRunOperationID = _OPERATION_ADJUST_;
		srEventMenuItem->inRunTRTID = _TRT_ADJUST_;
	}

	if (ginDebug == VS_TRUE)
		inDISP_LogPrintf("inMENU_SALE_ADJUST 5");

	return (inRetVal);

}

/*
Function        :inMENU_REDEEM_ADJUST
Date&Time       :2017/11/8 上午 10:08
Describe        :
*/
int inMENU_REDEEM_ADJUST(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_REDEEM_ADJUST_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利調帳＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _REDEEM_ADJUST_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_REDEEM_ADJUST_;
		srEventMenuItem->inRunTRTID = _TRT_REDEEM_ADJUST_;
		srEventMenuItem->uszRedeemBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_INST_ADJUST
Date&Time       :2017/11/8 上午 10:08
Describe        :
*/
int inMENU_INST_ADJUST(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_INST_ADJUST_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期調帳＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _INST_ADJUST_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}


		srEventMenuItem->inRunOperationID = _OPERATION_INST_ADJUST_;
		srEventMenuItem->inRunTRTID = _TRT_INST_ADJUST_;
		srEventMenuItem->uszInstallmentBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_FISC_MENU
Date&Time       :2017/8/24 下午 3:33
Describe        :
*/
int inMENU_FISC_MENU(EventMenuItem *srEventMenuItem)
{
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_8X16_OPT_;
	char			szKey = 0x00;
	char			szFiscFunctionEnable[2 + 1] = {0};
	char			szDemoMode[2 + 1] = {0};
//	TRANSACTION_OBJECT	pobTran;	/* 只用來看有沒有安全認證，無實際用途 */

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SMARTPAY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第一層顯示 Smartpay */

	/* 若CFGT的FiscFunctionEnable 和 MACEnable 未開，顯示此功能以關閉 */
	memset(szFiscFunctionEnable, 0x00, sizeof(szFiscFunctionEnable));
	inGetFiscFuncEnable(szFiscFunctionEnable);

	/* 沒開Fisc */
	if ((memcmp(&szFiscFunctionEnable[0], "Y", 1) != 0))
	{
		/* 此功能已關閉 */
		inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

		return (VS_ERROR);
	}
	else
	{
		/* 教育訓練模式 */
		memset(szDemoMode, 0x00, sizeof(szDemoMode));
		inGetDemoMode(szDemoMode);
		if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
		{

		}
		else
		{
/* 因為不需要在這邊換KEY,所以拿掉 20190328 [SAM] */
#if 0		
			/* SMARTPAY要GEN MAC來算TCC，一定要安全認證 */
			if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
			{
				memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
				if (inNCCC_Func_CUP_PowerOn_LogOn(&pobTran) != VS_SUCCESS)
				{
					/* 安全認證失敗 */
					inRetVal = VS_ERROR;
					return (inRetVal);
				}
			}
#endif
		}

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_MENU_FISC_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
	}


	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* 消費扣款 */
		if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
		{
			inRetVal = inMENU_FISC_SALE(srEventMenuItem);
			break;
		}
		/* 消費扣款沖正 */
		else if (szKey == _KEY_2_			||
			 inChoice == _OPTTouch8X16_LINE_6_)
		{
			inRetVal = inMENU_FISC_VOID(srEventMenuItem);
			break;
		}
		/* 退費 */
		else if (szKey == _KEY_3_			||
			 inChoice == _OPTTouch8X16_LINE_7_)
		{
			inRetVal = inMENU_FISC_REFUND(srEventMenuItem);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}

/*
Function        :inMENU_FISC_SALE
Date&Time       :2017/10/27 下午 3:13
Describe        :
*/
int inMENU_FISC_SALE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_FISC_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _FISC_SALE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
		srEventMenuItem->inRunTRTID = _TRT_FISC_SALE_CTLS_;
		srEventMenuItem->uszFISCTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_FISC_VOID
Date&Time       :2017/10/27 下午 3:26
Describe        :
*/
int inMENU_FISC_VOID(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_FISC_VOID_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款沖正＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _FISC_VOID_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_VOID_;
		srEventMenuItem->inRunTRTID = _TRT_FISC_VOID_;
		srEventMenuItem->uszFISCTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_FISC_REFUND
Date&Time       :2017/10/27 下午 3:26
Describe        :
*/
int inMENU_FISC_REFUND(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szCTLSEnable[2 + 1];

	if (inCREDIT_CheckTransactionFunction(_FISC_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);

		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退費交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _FISC_REFUND_;
		srEventMenuItem->uszFISCTransBit = VS_TRUE;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
		{
			srEventMenuItem->inRunOperationID = _OPERATION_FISC_REFUND_CTLS_;
			srEventMenuItem->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
		}
		else
		{
			srEventMenuItem->inRunOperationID = _OPERATION_FISC_REFUND_;
			srEventMenuItem->inRunTRTID = _TRT_FISC_REFUND_ICC_;
		}
	}

	return (inRetVal);
}

/*
Function        :inMENU_CUP_Sale
Date&Time       :2017/10/26 下午 5:53
Describe        :
*/
int inMENU_CUP_SALE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal= VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_CUP_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_SALE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_SALE_CTLS_;
		srEventMenuItem->inRunTRTID = _TRT_CUP_SALE_;
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_CUP_Void
Date&Time       :2017/10/26 下午 5:53
Describe        :
*/
int inMENU_CUP_VOID(EventMenuItem *srEventMenuItem)
{
	int	inRetVal= VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_CUP_VOID_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CUP_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯取消＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_VOID_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_VOID_;
		srEventMenuItem->inRunTRTID = _TRT_CUP_VOID_;
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function	:inMENU_CUP_Refund
Date&Time	:2017/6/7 下午 1:12
Describe	:銀聯退貨
*/
int inMENU_CUP_REFUND(EventMenuItem *srEventMenuItem)
{
	int			inRetVal= VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _CUP_REFUND_			, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_2_2_1_CUP_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _CUP_MAIL_ORDER_REFUND_	, inCREDIT_CheckTransactionFunction	, _ICON_HIGHTLIGHT_2_2_2_CUP_MAILORDER_REFUND_	},
		{_Touch_NONE_				, _TRANS_TYPE_NULL_		, NULL						, ""						}
	};

	if (inCREDIT_CheckTransactionFunction(_CUP_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯退貨> */
		inDISP_PutGraphic(_MENU_REFUND_OPTION_4_, 0,  _COORDINATE_Y_LINE_8_4_);

		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			/* 銀聯退貨 */
			if (szKey == _KEY_1_			||
			    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
			{
				inRetVal = inMENU_CUP_REFUND_NORMAL(srEventMenuItem);
				if (inRetVal == VS_USER_CANCEL)
				{
					inRetVal = VS_ERROR;
					break;
				}
				else if (inRetVal == VS_FUNC_CLOSE_ERR)
				{
					inRetVal = VS_SUCCESS;
				}
				else
				{
					break;
				}

			}
			/* 銀聯郵購退貨 */
			else if (szKey == _KEY_2_			||
				 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
			{
				inRetVal = inMENU_CUP_REFUND_MAILORDER(srEventMenuItem);
				if (inRetVal == VS_USER_CANCEL)
				{
					inRetVal = VS_ERROR;
					break;
				}
				else if (inRetVal == VS_FUNC_CLOSE_ERR)
				{
					inRetVal = VS_SUCCESS;
				}
				else
				{
					break;
				}
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}

		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}

	return (inRetVal);
}

/*
Function        :inMENU_CUP_REFUND_NORMAL
Date&Time       :2017/11/7 下午 4:54
Describe        :
*/
int inMENU_CUP_REFUND_NORMAL(EventMenuItem *srEventMenuItem)
{
	int	inRetVal= VS_SUCCESS;
	char	szCTLSEnable[2 + 1];

	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);

	if (inCREDIT_CheckTransactionFunction(_CUP_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯退貨> */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
		{
			srEventMenuItem->inRunOperationID = _OPERATION_REFUND_CTLS_CUP_;
			srEventMenuItem->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
		}
		else
		{
			srEventMenuItem->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
			srEventMenuItem->inRunTRTID = _TRT_CUP_REFUND_;
		}
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_CUP_REFUND_MAILORDER
Date&Time       :2017/11/7 下午 4:56
Describe        :
*/
int inMENU_CUP_REFUND_MAILORDER(EventMenuItem *srEventMenuItem)
{
	int	inRetVal= VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_CUP_MAIL_ORDER_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯退貨> */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_MAIL_ORDER_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_REFUND_;
		srEventMenuItem->inRunTRTID = _TRT_CUP_MAIL_ORDER_REFUND_;
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
		srEventMenuItem->uszMailOrderBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function	:inMENU_CUP_PreAuth
Date&Time	:2017/6/7 下午 1:12
Describe	:銀聯預先授權
*/
int inMENU_CUP_PREAUTH(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char	szKey = 0x00;

	if (inCREDIT_CheckTransactionFunction(_CUP_PRE_AUTH_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_MENU_PRE_AUTH_OPTION_2_, 0,  _COORDINATE_Y_LINE_8_4_);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			/* 預先授權 */
			if (szKey == _KEY_1_			||
			    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */
				/* 輸入密碼的層級 */
				srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
				srEventMenuItem->inCode = _CUP_PRE_AUTH_;

				/* 第一層輸入密碼 */
				if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
					break;
				}

				srEventMenuItem->inRunOperationID = _OPERATION_PRE_AUTH_;
				//srEventMenuItem->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				srEventMenuItem->inRunTRTID = _TRT_CUP_PRE_AUTH_;
				/* 標示是CUP交易 */
				srEventMenuItem->uszCUPTransBit = VS_TRUE;

				inRetVal = VS_SUCCESS;
				break;
			}
			/* 預先授權取消 */
			else if (szKey == _KEY_2_			||
				 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權取消＞ */
				/* 輸入密碼的層級 */
				srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
				srEventMenuItem->inCode = _CUP_PRE_AUTH_VOID_;

				/* 第一層輸入密碼 */
				if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
					break;
				}

				srEventMenuItem->inRunOperationID = _OPERATION_VOID_;
				srEventMenuItem->inRunTRTID = _TRT_CUP_PRE_AUTH_VOID_;
				/* 標示是CUP交易 */
				srEventMenuItem->uszCUPTransBit = VS_TRUE;

				inRetVal = VS_SUCCESS;
				break;
			}
			/* 預先授權完成 */
			else if (szKey == _KEY_3_			||
				 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */
				/* 輸入密碼的層級 */
				srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
				srEventMenuItem->inCode = _CUP_PRE_COMP_;

				/* 第一層輸入密碼 */
				if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
					return (VS_ERROR);

				srEventMenuItem->inRunOperationID = _OPERATION_PRE_COMP_;
				//srEventMenuItem->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				srEventMenuItem->inRunTRTID = _TRT_CUP_PRE_COMP_;
				/* 標示是CUP交易 */
				srEventMenuItem->uszCUPTransBit = VS_TRUE;

				inRetVal = VS_SUCCESS;
				break;
			}
			/* 授權完成 */
			else if (szKey == _KEY_4_			||
				 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成取消＞ */
				/* 輸入密碼的層級 */
				srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
				srEventMenuItem->inCode = _CUP_PRE_COMP_VOID_;

				/* 第一層輸入密碼 */
				if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
					break;
				}

				srEventMenuItem->inRunOperationID = _OPERATION_VOID_;
				srEventMenuItem->inRunTRTID = _TRT_CUP_PRE_COMP_VOID_;
				/* 標示是CUP交易 */
				srEventMenuItem->uszCUPTransBit = VS_TRUE;

				inRetVal = VS_SUCCESS;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}

		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}

        return (inRetVal);
}

/*
Function        :inMENU_CUP_MailOrder
Date&Time       :2017/10/26 下午 5:57
Describe        :
*/
int inMENU_CUP_MAILORDER(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_CUP_MAIL_ORDER_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_MAIL_ORDER_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜郵購交易＞ */

		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_MAIL_ORDER_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_MAIL_ORDER_;
		srEventMenuItem->inRunTRTID = _TRT_CUP_MAIL_ORDER_;
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
		srEventMenuItem->uszMailOrderBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_CUP_INST_REDEEM
Date&Time       :2017/10/26 下午 5:57
Describe        :
*/
int inMENU_CUP_INST_REDEEM(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REDEEM_INST_TITTLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期紅利＞ */
	/* 此功能已關閉 */
	inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);

	return (inRetVal);
}

/*
Function        :inMENU_CUP_INST
Date&Time       :2017/10/27 下午 1:45
Describe        :
*/
int inMENU_CUP_INST(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_CUP_INST_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期交易＞ */

		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_INST_SALE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
		srEventMenuItem->inRunTRTID = _TRT_CUP_INST_SALE_CTLS_;
		srEventMenuItem->uszInstallmentBit = VS_TRUE;
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_CUP_REDEEM
Date&Time       :2017/10/27 下午 1:45
Describe        :
*/
int inMENU_CUP_REDEEM(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	if (inCREDIT_CheckTransactionFunction(_CUP_REDEEM_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利交易＞ */

		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_REDEEM_SALE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_REDEEM_SALE_CTLS_;
		srEventMenuItem->inRunTRTID = _TRT_CUP_REDEEM_SALE_CTLS_;
		srEventMenuItem->uszRedeemBit = VS_TRUE;
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
	}

	return (inRetVal);
}

/*
Function        :inMENU_CUP_LOGON
Date&Time       :2017/10/30 下午 6:00
Describe        :
*/
int inMENU_CUP_LOGON(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	/*{
		int i,bAppNum = 3;
		unsigned char	uszKey;
		char		szTemplate[50 + 1];

		//while (1)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

			for (i = 0; i < bAppNum; i++)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				switch(i)
				{
					case 0:
						sprintf(szTemplate, "%d.%s", i + 1, "UICC CREDIT");
						break;
					case 1:
						sprintf(szTemplate, "%d.%s", i + 1, "UICC DEBIT");
						break;
					case 2:
						sprintf(szTemplate, "%d.%s", i + 1, "UICC QUASICREDIT");
						break;
				}
				inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_4_ + i, _DISP_LEFT_);
			}

			uszKey = uszKBD_GetKey(30);

		}

	}*/

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_LOGON_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第一層顯示 <安全認證> */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	/* 參數下載 */
	srEventMenuItem->inRunOperationID = _OPERATION_CUP_LOGON_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_REPRINT
Date&Time       :2017/10/30 下午 2:39
Describe        :
*/
int inMENU_REPRINT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜重印簽單＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_REPRINT_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_REVIEW
Date&Time       :2017/10/30 下午 4:36
Describe        :
*/
int inMENU_REVIEW(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <交易查詢> */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_REVIEW_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_TOTAL_REVIEW
Date&Time       :2017/10/30 下午 4:36
Describe        :總額查詢
*/
int inMENU_TOTAL_REVIEW(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <交易查詢> */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_REVIEW_TOTAL_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_DETAIL_REVIEW
Date&Time       :2017/10/30 下午 4:36
Describe        :明細查詢
*/
int inMENU_DETAIL_REVIEW(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <交易查詢> */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_REVIEW_DETAIL_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_TMS_PARAMETER_PRINT
Date&Time       :2017/10/30 下午 6:10
Describe        :
*/
int inMENU_TMS_PARAMETER_PRINT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_PRINT_TMS_REPORT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數列印＞ */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	/* 列印參數表 */
	srEventMenuItem->inRunOperationID = _OPERATION_FUN7_PRINT_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_DCC_PARAMETER_PRINT
Date&Time       :2017/10/30 下午 5:49
Describe        :
*/
int inMENU_DCC_PARAMETER_PRINT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_PRINT_DCC_REPORT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC參數列印＞ */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	/* 列印參數表 */
	srEventMenuItem->inRunOperationID = _OPERATION_FUN7_DCC_PRINT_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_TOTAL_REPORT
Date&Time       :2017/10/30 下午 4:30
Describe        :
*/
int inMENU_TOTAL_REPORT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TOTAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 總額報表 */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_TOTAL_REPORT_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_DETAIL_REPORT
Date&Time       :2017/10/30 下午 4:34
Describe
 * 明細報表列印用 目前使用中  2022/10/21 [SAM]
*/
int inMENU_DETAIL_REPORT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜明細報表＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_DETAIL_REPORT_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_SAM_REGISTER
Date&Time       :2017/10/30 下午 4:53
Describe        :
*/
int inMENU_SAM_REGISTER(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TSAM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜註冊SAM卡＞ */

	inDISP_ChineseFont("1.NCCC TSAM註冊", _FONTSIZE_12X19_, _LINE_16_6_, _DISP_LEFT_);
	inDISP_ChineseFont("2.票證 SAM註冊", _FONTSIZE_12X19_, _LINE_16_7_, _DISP_LEFT_);

	uszKey = 0;

	while (1)
	{
		uszKey = uszKBD_GetKey(_EDC_TIMEOUT_);

		/* NCCC tSAM註冊 */
		if (uszKey == _KEY_1_)
		{
			/* 輸入密碼的層級 */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;

			/* 第一層輸入密碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				return (VS_ERROR);

			srEventMenuItem->inRunOperationID = _OPERATION_TSAM_;
			srEventMenuItem->inRunTRTID = FALSE;
			break;
		}
		/* 票證SAM卡註冊 */
		else if (uszKey == _KEY_2_)
		{
			/* 輸入密碼的層級 */
			srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
			srEventMenuItem->inCode = FALSE;

			/* 第一層輸入密碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				return (VS_ERROR);

			srEventMenuItem->inRunOperationID = _OPERATION_TICKET_SAM_REGISTER_;
			srEventMenuItem->inRunTRTID = FALSE;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}

	return (inRetVal);
}

/*
Function        :inMENU_EDIT_PASSWORD
Date&Time       :2017/10/30 下午 5:25
Describe        :設定管理號碼
*/
int inMENU_EDIT_PASSWORD(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_PWD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 設定管理號碼 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inFunc_EditPWD_Flow();

	return (inRetVal);
}

/*
Function        :inMENU_TRACELOG_UP
Date&Time       :2017/10/30 下午 5:30
Describe        :
*/
int inMENU_TRACELOG_UP(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_RETURN_PARAM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 資訊回報 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	srEventMenuItem->inRunOperationID = _OPERATION_FUN6_TMS_TRACELOG_UP_;

	return (inRetVal);
}

/*
Function        :inMENU_CHECK_VERSION
Date&Time       :2017/10/30 下午 5:43
Describe        :
*/
int inMENU_CHECK_VERSION(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_AP_VERSION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜版本查詢> */
	srEventMenuItem->inPasswordLevel =  _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	
	
	inRetVal = inFunc_Check_Version_ID();

	return (inRetVal);
}

/*
Function        :inMENU_TIME_SETTING
Date&Time       :2017/10/30 下午 6:02
Describe        :
*/
int inMENU_TIME_SETTING(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_DATE_TIME_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜時間設定＞ */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inFunc_Fun3EditDateTime();

	return (inRetVal);
}

/*
Function        :inMENU_COMM_SETTING
Date&Time       :2017/10/30 下午 5:56
Describe        :
*/
int inMENU_COMM_SETTING(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 <通訊設定> */

	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;

	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	
	inRetVal = inCOMM_Fun3_SetCommWay();

	return (inRetVal);
}

/*
Function        :inMENU_TMS_PARAMETER_DOWNLOAD
Date&Time       :2017/10/30 下午 6:00
Describe        :
*/
int inMENU_TMS_PARAMETER_DOWNLOAD(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	/* TMS參數下載 */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	/* 參數下載 */
	srEventMenuItem->inRunOperationID = _OPERATION_FUN5_TMS_DOWNLOAD_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_DCC_PARAMETER_DOWNLOAD
Date&Time       :2017/10/30 下午 6:04
Describe        :
*/
int inMENU_DCC_PARAMETER_DOWNLOAD(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DCC_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC參數下載＞ */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	/* 參數下載 */
	srEventMenuItem->inRunOperationID = _OPERATION_FUN5_DCC_DOWNLOAD_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_TMS_TASK_REPORT
Date&Time       :2017/10/30 下午 6:07
Describe        :至現回報
*/
int inMENU_TMS_TASK_REPORT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TMS_TASK_MENU_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜至現回報＞ */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	srEventMenuItem->inRunOperationID = _OPERATION_FUN5_TMS_TASK_REPORT_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_DELETE_BATCH
Date&Time       :2018/5/22 下午 4:02
Describe        :清除批次
*/
int inMENU_DELETE_BATCH(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_RESET_BATCH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <清除批次> */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	srEventMenuItem->inRunOperationID = _OPERATION_DELETE_BATCH_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_DEBUG_SWITCH
Date&Time       :2018/5/22 下午 4:14
Describe        :
*/
int inMENU_DEBUG_SWITCH(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DEBUG_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜DEBUG開關＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	inFunc_DebugSwitch();

	return (inRetVal);
}

/*
Function        :inMENU_EDIT_TMEP_VERSION_ID
Date&Time       :2018/5/22 下午 4:24
Describe        :
*/
int inMENU_EDIT_TMEP_VERSION_ID(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_EDIT_TMS_VERSION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜修改TMS版本＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	inFunc_Set_Temp_VersionID();

	return (inRetVal);
}

/*
Function        :inMENU_EDIT_TMSOK
Date&Time       :2018/5/22 下午 4:51
Describe        :
*/
int inMENU_EDIT_TMSOK(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_EDIT_TMS_VERSION_OK_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜修改TMS完成＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_MANAGER_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	inFunc_Set_TMSOK_Flow();

	return (inRetVal);
}

/*
Function        :inMENU_UNLOCK_EDC
Date&Time       :2018/5/22 下午 4:56
Describe        :
*/
int inMENU_UNLOCK_EDC(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	/* 第三層顯示 ＜解鎖EDC＞ */
	inDISP_ChineseFont("解鎖EDC", _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	inFunc_Unlock_EDCLock_Flow();

	return (inRetVal);
}

/*
Function        :inMENU_REBOOT
Date&Time       :2018/5/22 下午 5:14
Describe        :
*/
int inMENU_REBOOT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REBOOT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜重新開機＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	srEventMenuItem->inRunOperationID = _OPERATION_EDC_REBOOT_;
	srEventMenuItem->inRunTRTID = FALSE;

	inFunc_Reboot();

	return (inRetVal);
}

/*
Function        :inMENU_DOWNLOAD_CUP_TEST_KEY
Date&Time       :2018/5/22 下午 5:19
Describe        :
*/
int inMENU_DOWNLOAD_CUP_TEST_KEY(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DOWNLOAD_CUP_KEY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜下載銀聯測試key＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

//	inNCCC_TMK_Write_Test_TerminalMasterKey_Flow();

	return (inRetVal);
}

/*
Function        :inMENU_EXIT_AP
Date&Time       :2018/5/22 下午 5:25
Describe        :
*/
int inMENU_EXIT_AP(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;

	inDISP_ClearAll();
	/* 第三層顯示 ＜離開程式＞ */
	inDISP_ChineseFont("離開程式", _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	inFunc_Exit_AP();

	return (inRetVal);
}

/*
Function        :inMENU_CHECK_FILE
Date&Time       :2018/5/22 下午 5:26
Describe        :
*/
int inMENU_CHECK_FILE(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;
	unsigned char	uszDispBit = VS_FALSE;
	
	inDISP_ClearAll();
	/* 第三層顯示 ＜確認檔案＞ */
	inDISP_ChineseFont("確認檔案", _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	while(1)
	{
		if (uszDispBit == VS_FALSE)
		{
			uszDispBit = VS_TRUE;
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("1.From SD", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
			inDISP_ChineseFont("2.From USB", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		}
		
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inMENU_CHECK_FILE_In_SD(srEventMenuItem);
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_2_)
		{
			inMENU_CHECK_FILE_In_USB(srEventMenuItem);
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}


/*
Function        :inMENU_CHECK_FILE_In_SD
Date&Time       :2019/2/18 上午 9:49
Describe        :
*/
int inMENU_CHECK_FILE_In_SD(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;
	unsigned char	uszDispBit = VS_FALSE;
	
	while(1)
	{
		if (uszDispBit == VS_FALSE)
		{
			uszDispBit = VS_TRUE;
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("SD", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
			inDISP_ChineseFont("1.Check Partial", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
			inDISP_ChineseFont("2.Check ALL", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		}
		
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_6_);
			inFunc_CheckFile_In_SD_Partial();
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_2_)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_6_);
			inFunc_CheckFile_In_SD_ALL();
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}


/*
Function        :inMENU_CHECK_FILE_In_USB
Date&Time       :2019/2/18 上午 9:49
Describe        :
*/
int inMENU_CHECK_FILE_In_USB(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;
	unsigned char	uszDispBit = VS_FALSE;
	
	while(1)
	{
		if (uszDispBit == VS_FALSE)
		{
			uszDispBit = VS_TRUE;
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("USB", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
			inDISP_ChineseFont("1.Check Partial", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
			inDISP_ChineseFont("2.Check ALL", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		}
		
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_6_);
			inFunc_CheckFile_In_USB_Partial();
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_2_)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_6_);
			inFunc_CheckFile_In_USB_ALL();
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}


/*
Function        :inMENU_Engineer_Fuction
Date&Time       :2018/7/27 上午 9:31
Describe        :
*/
int inMENU_Engineer_Fuction(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;

	inDISP_ClearAll();
	/* 第三層顯示 ＜確認檔案＞ */
	inDISP_ChineseFont("工程師功能頁面", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_MANAGER_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("1.切換ECR Comport", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);

	while(1)
	{
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inFunc_ECR_Comport_Switch();
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			break;
		}
		else
		{

		}
	}

	return (inRetVal);
}

/*
Function        :inMENU_Check_Adjust
Date&Time       :2017/11/6 下午 5:32
Describe        :檢查是否全部調帳功能都不能用
*/
int inMENU_Check_Adjust(int inCode)
{
	char		szTransFunc[20 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;

	/* 只檢查NCCC的功能 */
	if (inLoadHDTRec(0) < 0)
	{
		inDISP_DispLogAndWriteFlie(" MENU Check Adj Load HDT 0 *Error* Line[%d]", __LINE__);
	}

	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);

	/* 修改TMS 20190211 [SAM] */
	if(szTransFunc[10] != 0x31 && szTransFunc[11] != 0x31)
	{
		uszTxnEnable = VS_FALSE;
	}

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_Transaction_Enable
Date&Time       :2018/2/5 下午 2:42
Describe        :  交易選單，檢查是否有下TMS，如果有下TMS就可以開啟功能
*/
int inMENU_Check_Transaction_Enable(int inCode)
{
	char		szFunEnable[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;

	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetTMSOK(szFunEnable);

	/* 沒下TMS */
	if (memcmp(&szFunEnable[0], "Y", 1) != 0)
	{
		uszTxnEnable = VS_FALSE;
	}
	/* 有下TMS */
	else
	{
		uszTxnEnable = VS_TRUE;
	}

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_CUP_Enable
Date&Time       :2017/11/3 下午 2:42
Describe        : 檢查銀聯功能是否可進行
*/
int inMENU_Check_CUP_Enable(int inCode)
{
	char		szCUPFunctionEnable[2 + 1];
	char		szMACEnable[2 + 1];
	char		szTMSOK[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;

	/* 若EDC的CUPFunctionEnable 和 MACEnable 未開，顯示此功能以關閉 */
	memset(szCUPFunctionEnable, 0x00, sizeof(szCUPFunctionEnable));
	inGetCUPFuncEnable(szCUPFunctionEnable);
	memset(szMACEnable, 0x00, sizeof(szMACEnable));
	inGetMACEnable(szMACEnable);

	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);

	/* 沒開CUP */
	if ((memcmp(&szCUPFunctionEnable[0], "Y", 1) != 0)	||
	    (memcmp(szMACEnable, "Y", 1) != 0)			||
	    (memcmp(szTMSOK, "Y", 1) != 0))
	{
		uszTxnEnable = VS_FALSE;
	}
	/* 有開CUP */
	else
	{
		uszTxnEnable = VS_TRUE;
	}

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_SMARTPAY_Enable
Date&Time       :2017/11/3 下午 2:42
Describe        :
*/
int inMENU_Check_SMARTPAY_Enable(int inCode)
{
	char		szFiscFunctionEnable[2 + 1];
	char		szTMSOK[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;

	/* 若CFGT的FiscFunctionEnable 和 MACEnable 未開，顯示此功能以關閉 */
	memset(szFiscFunctionEnable, 0x00, sizeof(szFiscFunctionEnable));
	inGetFiscFuncEnable(szFiscFunctionEnable);

	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);

	/* 沒開Fisc */
	if ((memcmp(&szFiscFunctionEnable[0], "Y", 1) != 0)	||
	    (memcmp(szTMSOK, "Y", 1) != 0))
	{
		uszTxnEnable = VS_FALSE;
	}
	else
	{
		uszTxnEnable = VS_TRUE;
	}

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        : inMENU_Check_ETICKET_Enable
Date&Time   : 2017/11/3 下午 2:42
Describe        : inCode沒有使用
*/
int inMENU_Check_ETICKET_Enable(int inCode)
{
	int		inESVCIndex = -1;
	char		szETICKETEnable[2 + 1];
	char		szTMSOK[2 + 1];
	unsigned char	uszTxnEnable = VS_FALSE;

	inFunc_Find_Specific_HDTindex(0, _HOST_NAME_ESVC_, &inESVCIndex);

	if (inESVCIndex != -1)
	{
		memset(szTMSOK, 0x00, sizeof(szTMSOK));
		inGetTMSOK(szTMSOK);

		if (memcmp(szTMSOK, "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
		else
		{
			if (inLoadHDTRec(inESVCIndex) == VS_ERROR)
			{
				inDISP_DispLogAndWriteFlie(" MENU Check ETicket Load HDT[%d] *Error* Line[%d]", inESVCIndex, __LINE__);
			}
			
			memset(szETICKETEnable, 0x00, sizeof(szETICKETEnable));

			if (uszTxnEnable == VS_FALSE)
			{
				inLoadTDTRec(_TDT_INDEX_00_IPASS_);
				memset(szETICKETEnable, 0x00, sizeof(szETICKETEnable));
				inGetTicket_HostEnable(szETICKETEnable);
				if (memcmp(szETICKETEnable, "Y", strlen("Y")) == 0)
				{
					uszTxnEnable = VS_TRUE;
				}
			}

			if (uszTxnEnable == VS_FALSE)
			{
				inLoadTDTRec(_TDT_INDEX_01_ECC_);
				memset(szETICKETEnable, 0x00, sizeof(szETICKETEnable));
				inGetTicket_HostEnable(szETICKETEnable);
				if (memcmp(szETICKETEnable, "Y", strlen("Y")) == 0)
				{
					uszTxnEnable = VS_TRUE;
				}
			}

			if (uszTxnEnable == VS_FALSE)
			{
				inLoadTDTRec(_TDT_INDEX_02_ICASH_);
				memset(szETICKETEnable, 0x00, sizeof(szETICKETEnable));
				inGetTicket_HostEnable(szETICKETEnable);
				if (memcmp(szETICKETEnable, "Y", strlen("Y")) == 0)
				{
					uszTxnEnable = VS_TRUE;
				}
			}

			if (uszTxnEnable == VS_FALSE)
			{
				inLoadTDTRec(_TDT_INDEX_03_HAPPYCASH_);
				memset(szETICKETEnable, 0x00, sizeof(szETICKETEnable));
				inGetTicket_HostEnable(szETICKETEnable);
				if (memcmp(szETICKETEnable, "Y", strlen("Y")) == 0)
				{
					uszTxnEnable = VS_TRUE;
				}
			}
		}
	}
	else
	{
		uszTxnEnable = VS_FALSE;
	}

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_AWARD_Enable
Date&Time       :2017/11/3 下午 6:09
Describe        :
*/
int inMENU_Check_AWARD_Enable(int inCode)
{
	char		szTMSOK[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;
	unsigned char	uszCreditCardSupport = VS_FALSE;
	unsigned char	uszBarcodeSupport = VS_FALSE;
	unsigned char	uszVoidRedeemSupport = VS_FALSE;
	unsigned char	uszASMSupport = VS_FALSE;

	/* 判斷是否支援過卡兌換 */
//	if (inNCCC_Loyalty_CreditCardFlag(0) == VS_SUCCESS)
	if(1)
	{
		uszCreditCardSupport = VS_TRUE;
	}
	else
	{
		uszCreditCardSupport = VS_FALSE;
	}

	/* 判斷是否支援條碼兌換 */
//	if (inNCCC_Loyalty_BarCodeFlag(0) == VS_SUCCESS)
	if(1)
	{
		uszBarcodeSupport = VS_TRUE;
	}
	else
	{
		uszBarcodeSupport = VS_FALSE;
	}

	/* 判斷是否支援條碼兌換取消 */
//	if (inNCCC_Loyalty_VoidRedeemFlag(0) == VS_SUCCESS)
	if(1)
	{
		uszVoidRedeemSupport = VS_TRUE;
	}
	else
	{
		uszVoidRedeemSupport = VS_FALSE;
	}

	/* 判斷是否支援優惠平台(含詢問電文) */
//	if (inNCCC_Loyalty_ASM_Flag() == VS_SUCCESS)
	if(1)
	{
		uszASMSupport = VS_TRUE;
	}
	else
	{
		uszASMSupport = VS_FALSE;
	}

	/*	兌換方式
		‘1’=以條碼當作兌換資訊，透過收銀機條碼資訊。
		‘2’=以條碼當作兌換資訊，端末機接BarCode Reader掃描兌換(核銷)條碼。
		‘3’=以條碼當作兌換資訊，手動於端末機輸入兌換(核銷)條碼。
		‘4’=以卡號當作兌換資訊，於端末機上刷卡。
		‘5’=以卡號當作兌換資訊，於端末機上手動輸入。
	 */

	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);

	/* 若全部開關沒開，直接顯示此功能以關閉 */
	if ((uszCreditCardSupport != VS_TRUE && uszBarcodeSupport != VS_TRUE && uszVoidRedeemSupport != VS_TRUE && uszASMSupport != VS_TRUE)	||
	    (memcmp(szTMSOK, "Y", 1) != 0))
	{
		uszTxnEnable = VS_FALSE;
	}
	else
	{
		uszTxnEnable = VS_TRUE;
	}

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_AWARD_Enable
Date&Time       :2017/11/3 下午 6:15
Describe        :
*/
int inMENU_Check_HG_Enable(int inCode)
{
	char		szHostEnable[2 + 1];
	char		szTMSOK[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;

	memset(szHostEnable, 0x00, sizeof(szHostEnable));
//	inHG_GetHG_Enable(0, szHostEnable);

	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);

	if ((memcmp(szHostEnable, "Y", strlen("Y")) != 0)	||
	    (memcmp(szTMSOK, "Y", 1) != 0))
	{
		uszTxnEnable = VS_FALSE;
	}
	else
	{
		uszTxnEnable = VS_TRUE;
	}

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        : inMENU_Check_SETTLE_Enable
Date&Time   : 2018/2/5 下午 3:30
Describe        : 檢查是否可執行結帳功能
*/
int inMENU_Check_SETTLE_Enable(int inCode)
{
	int		i = 0;
	char		szFunEnable[2 + 1] = {0};
	char		szTransFunc[20 + 1] = {0};
	char		szHostLabel[8 + 1] = {0};
	unsigned char	uszTxnEnable = VS_FALSE;	/* 預設為失敗，只要其中一個有開就不反白 */

	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetTMSOK(szFunEnable);

	/* 沒下TMS */
	if (memcmp(szFunEnable, "Y", 1) != 0)
	{
		uszTxnEnable = VS_FALSE;
	}
	/* 有下TMS */
	else
	{
		/* 有下TMS才檢查開關 */
		/* 只要其中一個有開就不反白 */
		for (i = 0 ;; i ++)
		{
			if (inLoadHDTRec(i) < 0)	/* 主機參數檔【HostDef.txt】 */
			{
				inDISP_LogPrintfWithFlag(" MENU Check Settle Load HDT[%d] *Error* Line[%d]", i, __LINE__);
				break;
			}

			/* 沒開不檢查 */
			memset(szFunEnable, 0x00, sizeof(szFunEnable));
			inGetHostEnable(szFunEnable);
			if (szFunEnable[0] != 'Y')
			{
				continue;
			}

			memset(szHostLabel, 0x00, sizeof(szHostLabel));
			inGetHostLabel(szHostLabel);

			memset(szTransFunc, 0x00, sizeof(szTransFunc));
			inGetTransFunc(szTransFunc);

			if (memcmp(szHostLabel, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_)) == 0)
			{
				if (szTransFunc[2] == 0x31) /* 修改TMS交易開關位置 20190211 [SAM] */
				{
					uszTxnEnable = VS_TRUE;
					break;
				}
			}
			else if (memcmp(szHostLabel, _HOST_NAME_DINERS_, strlen(_HOST_NAME_DINERS_)) == 0)
			{
				if (szTransFunc[2] == 0x31) /* 修改TMS交易開關位置 20190211 [SAM] */
				{
					uszTxnEnable = VS_TRUE;
					break;
				}
			}
			else if (memcmp(szHostLabel, _HOST_NAME_AMEX_, strlen(_HOST_NAME_AMEX_)) == 0)
			{
				if (szTransFunc[2] == 0x31) /* 修改TMS交易開關位置 20190211 [SAM] */					
				{
					uszTxnEnable = VS_TRUE;
					break;
				}
			}
#ifdef __NCCC_DCC_FUNC__	
			else if (memcmp(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)) == 0)
			{
				if (szTransFunc[2] == 0x31) /* 修改TMS交易開關位置 20190211 [SAM] */
				{
					uszTxnEnable = VS_TRUE;
					break;
				}
			}
#endif			

			else if (memcmp(szHostLabel, _HOST_NAME_CREDIT_FUBON_, strlen(_HOST_NAME_CREDIT_FUBON_)) == 0)
			{
				if (szTransFunc[2] == 0x31) /* 修改TMS交易開關位置 20190211 [SAM] */
				{
					uszTxnEnable = VS_TRUE;
					break;
				}
			}
			else if (memcmp(szHostLabel, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
			{
				/* IPASS */
				inLoadTDTRec(_TDT_INDEX_00_IPASS_);
				memset(szFunEnable, 0x00, sizeof(szFunEnable));
				inGetTicket_HostEnable(szFunEnable);

				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
				{
					inLoadIPASSDTRec(0);
					memset(szTransFunc, 0x00, sizeof(szTransFunc));
					inGetIPASS_Transaction_Function(szTransFunc);

					if (szTransFunc[8] == 'Y')
					{
						uszTxnEnable = VS_TRUE;
						break;
					}
				}

				/* ECC */
				inLoadTDTRec(_TDT_INDEX_01_ECC_);
				memset(szFunEnable, 0x00, sizeof(szFunEnable));
				inGetTicket_HostEnable(szFunEnable);

				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
				{
					inLoadECCDTRec(0);
					memset(szTransFunc, 0x00, sizeof(szTransFunc));
					inGetECC_Transaction_Function(szTransFunc);

					if (szTransFunc[8] == 'Y')
					{
						uszTxnEnable = VS_TRUE;
						break;
					}
				}

//				/* ICASH */
//				inLoadTDTRec(_TDT_INDEX_02_ICASH_);
//				memset(szFunEnable, 0x00, sizeof(szFunEnable));
//				inGetTicket_HostEnable(szFunEnable);
//
//				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
//				{
////					inLoadECCDTRec(0);
//					memset(szTransFunc, 0x00, sizeof(szTransFunc));
////					inGetECC_Transaction_Function(szTransFunc);
//
//					if (szTransFunc[8] == 'Y')
//					{
//						uszTxnEnable = VS_TRUE;
//						break;
//					}
//				}

//				/* Happycash */
//				inLoadTDTRec(_TDT_INDEX_03_HAPPYCASH_);
//				memset(szFunEnable, 0x00, sizeof(szFunEnable));
//				inGetTicket_HostEnable(szFunEnable);
//
//				if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
//				{
////					inLoadECCDTRec(0);
//					memset(szTransFunc, 0x00, sizeof(szTransFunc));
////					inGetECC_Transaction_Function(szTransFunc);
//
//					if (szTransFunc[8] == 'Y')
//					{
//						uszTxnEnable = VS_TRUE;
//						break;
//					}
//				}
			}if (memcmp(szHostLabel, _HOST_NAME_SVC_, strlen(_HOST_NAME_SVC_)) == 0)
			{	/* [新增SVC功能]  檢查是否要進行結帳  2022/12/26 [SAM] */
				//TODO: 目前暫未定檢查項，要補。
				uszTxnEnable = VS_TRUE;
				break;
			}
		}
	}

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_REVIEW_SETTLE_Enable
Date&Time       :2018/2/5 下午 3:30
Describe        :  檢查是否可進行結帳功能
*/
int inMENU_Check_REVIEW_SETTLE_Enable(int inCode)
{
	char		szFunEnable[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;

	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetTMSOK(szFunEnable);

	/* 沒下TMS */
	if (memcmp(szFunEnable, "Y", 1) != 0)
	{
		uszTxnEnable = VS_FALSE;
	}
	/* 有下TMS */
	else
	{
		uszTxnEnable = VS_TRUE;
	}

	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_HG_Refund_Enable
Date&Time       :2017/11/3 下午 6:15
Describe        :
*/
int inMENU_Check_HG_Refund_Enable(int inCode)
{
	char		szHostEnable[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;

	memset(szHostEnable, 0x00, sizeof(szHostEnable));
//	inHG_GetHG_Enable(0, szHostEnable);
	if (memcmp(szHostEnable, "Y", strlen("Y")) != 0)
	{
		uszTxnEnable = VS_FALSE;
	}
	else
	{
		uszTxnEnable = VS_TRUE;
	}



	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Display_ICON
Date&Time       :2017/10/31 上午 10:01
Describe        :inKeyPostionID 用來標示是哪一個key，用以確認位置
*/
int inMENU_Display_ICON(char* szFileName, int inButtonPostionID)
{
	switch (inButtonPostionID)
	{
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_1_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_2_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_3_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_8_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_9_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y1_);
			break;

		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y1_);
			break;

		default:
			break;
	}

	return (VS_SUCCESS);
}

/*
Function        :inMENU_CHECK_FUNCTION_ENABLE_DISPLAY
Date&Time       :2017/10/31 下午 2:21
Describe        :
*/
int inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(MENU_CHECK_TABLE *srMENU_CHECK_TABLE)
{
	int	i = 0;

	while(1)
	{
		if (srMENU_CHECK_TABLE[i].inButtonPositionID == _Touch_NONE_)
		{
			break;
		}

		if (srMENU_CHECK_TABLE[i].inCheckFunc(srMENU_CHECK_TABLE[i].inCode) != VS_SUCCESS)
		{
			inMENU_Display_ICON(srMENU_CHECK_TABLE[i].szFileName, srMENU_CHECK_TABLE[i].inButtonPositionID);
		}

		i++;
	}

	return (VS_SUCCESS);
}


/*
Function        :inMENU_NEWUI_ETICKET_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
 * [新增電票悠遊卡功能] 重新說明，此功能為 NCCC 原有FUNCTION  [SAM]  
*/
int inMENU_NEWUI_ETICKET_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _TICKET_IPASS_DEDUCT_		, inNCCC_Ticket_Func_Check_Transaction_Deduct		, _ICON_HIGHTLIGHT_4_1_DEDUCT_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _TICKET_IPASS_REFUND_		, inNCCC_Ticket_Func_Check_Transaction_Refund		, _ICON_HIGHTLIGHT_4_2_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _TICKET_IPASS_INQUIRY_		, inNCCC_Ticket_Func_Check_Transaction_Inquiry		, _ICON_HIGHTLIGHT_4_3_INQUIRY_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _TICKET_IPASS_TOP_UP_		, inNCCC_Ticket_Func_Check_Transaction_Top_Up		, _ICON_HIGHTLIGHT_4_4_TOP_UP_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _TICKET_IPASS_VOID_TOP_UP_	, inNCCC_Ticket_Func_Check_Transaction_Void_Top_Up	, _ICON_HIGHTLIGHT_4_5_VOID_TOP_UP_	},

	};

	if (inMENU_Check_ETICKET_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 初始化 */
		inDISP_ClearAll();
		if (inNCCC_Ticket_Logon_ShowResult() != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
		inDISP_PutGraphic(_TOUCH_NEWUI_ETICKET_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
		inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		while (inPageLoop != _PAGE_LOOP_0_)
		{
			while (inPageLoop == _PAGE_LOOP_1_)
			{
				szKey = uszKBD_Key();
				inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				switch (inChioce1)
				{
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
						inCount = 1;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
						inCount = 2;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
						inCount = 3;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
						inCount = 4;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
						inCount = 5;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_RETURN_IDLE_BUTTON_:
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _NEWUI_LAST_PAGE_BUTTON_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _DisTouch_Slide_Down_To_Up_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					default:
						break;
				}

				if (szKey == _KEY_1_)
				{
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_2_)
				{
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_3_)
				{
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_4_)
				{
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_5_)
				{
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}

			}

			/* 判斷MENU點進去按鈕之後要做的事情 */
			if (inPageLoop == _PAGE_LOOP_2_)
			{
				switch (inCount)
				{
					case 1:
						inRetVal = inMENU_ETICKET_DEDUCT(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_ETICKET_REFUND(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 3:
						inRetVal = inMENU_ETICKET_INQUIRY(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 4:
						inRetVal = inMENU_ETICKET_TOP_UP(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 5:
						inRetVal = inMENU_ETICKET_VOID_TOP_UP(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}
			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
				inDISP_PutGraphic(_TOUCH_NEWUI_ETICKET_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
				inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

				inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			}
			/* 功能未開而返回，不刷畫面 */
			else if (inPageLoop == _PAGE_LOOP_1_	&&
				 inRetVal == VS_SUCCESS)
			{

			}
		}/* _PAGE_LOOP_0_ */
	}

	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_AWARD_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU_NEWUI_AWARD_MENU(EventMenuItem *srEventMenuItem)
{
	int		inPageLoop = _PAGE_LOOP_1_;
        int		inRetVal = VS_SUCCESS;
        int		inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int		inCount= 1;
        int		inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char		szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
//		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _LOYALTY_REDEEM_		, inNCCC_Loyalty_CreditCardFlag		, _ICON_HIGHTLIGHT_5_1_AWARD_SWIPE_	},
//		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _LOYALTY_REDEEM_		, inNCCC_Loyalty_BarCodeFlag		, _ICON_HIGHTLIGHT_5_2_AWARD_BARCODE_	},
//		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _VOID_LOYALTY_REDEEM_		, inNCCC_Loyalty_VoidRedeemFlag		, _ICON_HIGHTLIGHT_5_3_AWARD_REFUND_	},
//		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _LOYALTY_REDEEM_REFUND_	, inNCCC_Loyalty_RefundFlag		, _ICON_HIGHTLIGHT_5_4_AWARD_REDEEM_	},
//20190308		
		
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _LOYALTY_REDEEM_		, NULL		, _ICON_HIGHTLIGHT_5_1_AWARD_SWIPE_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _LOYALTY_REDEEM_		, NULL		, _ICON_HIGHTLIGHT_5_2_AWARD_BARCODE_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _VOID_LOYALTY_REDEEM_		, NULL		, _ICON_HIGHTLIGHT_5_3_AWARD_REFUND_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _LOYALTY_REDEEM_REFUND_	, NULL		, _ICON_HIGHTLIGHT_5_4_AWARD_REDEEM_	},
		{_Touch_NONE_				, _TRANS_TYPE_NULL_		, NULL					, ""					}
	};


	if (inMENU_Check_AWARD_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_5_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
		inDISP_PutGraphic(_TOUCH_NEWUI_AWARD_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
		inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		while (inPageLoop != _PAGE_LOOP_0_)
		{
			while (inPageLoop == _PAGE_LOOP_1_)
			{
				szKey = uszKBD_Key();
				inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				switch (inChioce1)
				{
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
						inCount = 1;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
						inCount = 2;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
						inCount = 3;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
						inCount = 4;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_RETURN_IDLE_BUTTON_:
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _NEWUI_LAST_PAGE_BUTTON_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _DisTouch_Slide_Down_To_Up_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					default:
						break;
				}

				if (szKey == _KEY_1_)
				{
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_2_)
				{
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_3_)
				{
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_4_)
				{
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}

			}

			/* 判斷MENU點進去按鈕之後要做的事情 */
			if (inPageLoop == _PAGE_LOOP_2_)
			{
				switch (inCount)
				{
					case 1:
						inRetVal = inMENU_AWARD_SWIPE(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_AWARD_BARCODE(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 3:
						inRetVal = inMENU_AWARD_VOID(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 4:
						inRetVal = inMENU_AWARD_REFUND(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}

			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_5_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
				inDISP_PutGraphic(_TOUCH_NEWUI_AWARD_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
				inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

				inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			}
			/* 功能未開而返回，不刷畫面 */
			else if (inPageLoop == _PAGE_LOOP_1_	&&
				 inRetVal == VS_SUCCESS)
			{

			}

		}/* _PAGE_LOOP_0_ */
	}

	return (inRetVal);
}
