#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Transaction.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/EDC.h"
#include "../EVENT/MenuMsg.h"

#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"

#include "../FUNCTION/ECR_FUNC/ECR.h"
#include "../FUNCTION/ECR_FUNC/RS232.h"

#include "../FUNCTION/MULTI_FUNC/MultiFunc.h"

#include "Display.h"
#include "DisTouch.h"

extern int ginTimeLogDebug;
extern int ginDebug;
int ginSetFont = -1;		/* 判斷目前SetFont */
extern int ginIdleDispFlag; /* 用來控制什麼時候要display Idle圖片，因為while迴圈內一直顯示圖片會造成觸控判斷延遲 */
extern MULTI_TABLE gstMultiOb;

int ginDispTimeOut = 0;
unsigned long gulDispTimeStart = 0;

/* 用來判斷是否要再接收ECR的資料，用在讀卡時的第二段交易 2022/2/18 [SAM] */
static int st_inStopReceiveEcr = VS_TRUE;

/* 每次開機會紀錄RTC換秒時的tick是多少，借以校正log毫秒*/
unsigned long gulTickCorrection = 0;

static char st_szMsgTemp[42];

/* 抓取用來判斷是否要再接收ECR的資料，用在讀卡時的第二段交易 2022/2/18 [SAM] */
int inGetStopEcrReceive()
{
	return st_inStopReceiveEcr;
}

/* 設定用來判斷是否要再接收ECR的資料，用在讀卡時的第二段交易 2022/2/18 [SAM] */
int inSetStopEcrReceive(int inStopReceive)
{
	st_inStopReceiveEcr = inStopReceive;
	return VS_SUCCESS;
}

/*
Function        :lnTimerStart
Date&Time       :2016/6/21 下午 1:54
Describe        :設定計時器開啟，inTimerNbr是第幾號計時器，lnDelay是多久Timeout
*/
int inTimerStart(int inTimerNbr, long lnDelay)
{
	CTOS_TimeOutSet(inTimerNbr, lnDelay * 100);

	return (VS_SUCCESS);
}

/*
Function        :inTimerStart_MicroSecond
Date&Time       :2018/2/9 下午 1:55
Describe        :設定計時器開啟，inTimerNbr是第幾號計時器，lnDelay是多久Timeout
 *		 lnDelay傳入單位為 10MicroSecond泛用度更高
*/
int inTimerStart_MicroSecond(int inTimerNbr, long lnDelay)
{
	CTOS_TimeOutSet(inTimerNbr, lnDelay);

	return (VS_SUCCESS);
}

/*
Function        :inTimerGet
Date&Time       :2016/6/21 下午 1:54
Describe        :確認計時器是否TimeOut，若timeout會回傳VS_SUCCESS
*/
int inTimerGet(int inTimerNbr)
{
	if (CTOS_TimeOutCheck(inTimerNbr) == d_YES)
		return (VS_SUCCESS);
	else
		return (VS_ERROR);
}

/*
Function        :uszSWIPE_Open
Date&Time       :2015/9/14 上午 11:00
Describe        :開刷卡槽控制權
*/
unsigned char uszSWIPE_Open(void)
{
	return (VS_TRUE);
}

/*
Function        :uszSWIPE_Close
Date&Time       :2015/9/14 上午 11:00
Describe        :關刷卡槽控制權
*/
unsigned char uszSWIPE_Close(void)
{
	return (VS_TRUE);
}

/*
Function        :uszFlushKBDBuffer
Date&Time       :2015/9/14 上午 11:00
Describe        :清磁條buffer
*/
unsigned char uszFlushSWIPEBuffer(void)
{
	return (VS_SUCCESS);
}

unsigned char uszKBD_Key(void)
{
	int inRetVal = VS_SUCCESS;
	unsigned char uszKey = 0;

	/* 判斷按鍵觸發，若觸發fKeyPressed為d_TRUE */
	inRetVal = inKBD_Key_IsPressed();

	if (inRetVal == VS_SUCCESS)
	{
		uszKey = uszKBD_Key_Hit();
		if (uszKey != 0)
		{
			return (uszKey);
		}
	}

	return uszKey;
}

/*
Function        :szKBD_Key_Keep
Date&Time       :2017/12/18 下午 6:08
Describe        :
*/
unsigned char szKBD_Key_Keep(void)
{
	int inRetVal = VS_SUCCESS;
	unsigned char uszKey = 0;

	/* 判斷按鍵觸發，若觸發fKeyPressed為d_TRUE */
	inRetVal = inKBD_Key_IsPressed();

	if (inRetVal == VS_SUCCESS)
	{
		uszKey = uszKBD_Key_In();
		if (uszKey != 0)
		{
			return (uszKey);
		}
	}

	return uszKey;
}

/*
Function        :szKBD_Key_Hit
Date&Time       :2017/12/18 下午 4:50
Describe        :偵測壓過的key並從Buffer移除
*/
unsigned char uszKBD_Key_Hit(void)
{
	unsigned char uszKey = 0;

	if (CTOS_KBDHit(&uszKey) == d_OK)
	{
		if (uszKey == d_KBD_INVALID)
		{
			uszKey = 0;
			return (uszKey);
		}
		else
		{
			return (uszKey);
		}
	}
	else
	{
		return (uszKey);
	}
}

/*
Function        :uszKBD_Key_In
Date&Time       :2017/12/18 下午 4:50
Describe        :偵測壓過的key並不從Buffer移除
*/
unsigned char uszKBD_Key_In(void)
{
	unsigned char uszKey = 0;

	if (CTOS_KBDInKeyCheck(&uszKey) == d_OK)
	{
		if (uszKey == d_KBD_INVALID)
		{
			uszKey = 0;
			return (uszKey);
		}
		else
		{
			return (uszKey);
		}
	}
	else
	{
		return (uszKey);
	}
}

/*
Function        :inKBD_Key_IsPressed
Date&Time       :2017/12/18 下午 5:05
Describe        :
*/
int inKBD_Key_IsPressed(void)
{
	VS_BOOL fKeyPressed = VS_FALSE;

	/* 判斷按鍵觸發，若觸發fKeyPressed為d_TRUE */
	CTOS_KBDInKey(&fKeyPressed);

	if (fKeyPressed == d_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

unsigned char uszKBD_Close(void)
{
	return (VS_TRUE);
}

unsigned char uszKBD_Open(unsigned char uszLs)
{
	return (VS_TRUE);
}

/*
Function	:szKBD_GetKey
Date&Time	:2016/8/19 上午 10:31
Describe	:到TimeOut時間到前，若有使用按鍵，則回傳按鍵，否則TimeOut 就回傳_KEY_TIMEOUT_
 *		另外，若傳進的時間小於0，則使用EDC.dat的EnterTimeOut
*/
unsigned char uszKBD_GetKey(int timeout) /* time out sec */
{
	int inFinalTimeOut;
	char szTimeOut[3 + 1];
	unsigned char uszKey;

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	if (timeout >= 0)
	{
		inFinalTimeOut = timeout;
	}
	else
	{
		memset(szTimeOut, 0x00, sizeof(szTimeOut));
		inGetEnterTimeout(szTimeOut);
		inFinalTimeOut = atoi(szTimeOut);
	}

	inDISP_Timer_Start(_TIMER_NEXSYS_2_, inFinalTimeOut);

	while (1)
	{
		/* 檢查TIMEOUT */
		if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
		{
			return (_KEY_TIMEOUT_);
		}

		uszKey = uszKBD_Key();

		if (uszKey != 0)
			break;
	}

	return (uszKey);
}

/*
Function        :inFlushKBDBuffer
Date&Time       :2015/9/14 上午 11:00
Describe        :清鍵盤buffer
*/
int inFlushKBDBuffer(void)
{
	CTOS_KBDBufFlush();
	return (VS_SUCCESS);
}

/*
Function	  : inDISP_ClearAll
Date&Time : 2015/6/7 下午 8:20
Describe	  : 顯示參數初始化
*/
int inDISP_Initial(void)
{
	/* 步驟一 設定LCD顯示畫面大小格式 */
	CTOS_LCDSelectMode(d_LCD_GRAPHIC_HIGH_RES_MODE);

	/* 步驟二 設定LCD顯示畫面字型格式 */
	CTOS_LCDFontSelectMode(d_FONT_TTF_MODE);

	/* 步驟三 設定TTF字型及Style */
	inDISP_TTF_SetFont(_DISP_CHINESE_, _FONT_REGULAR_); /* 微軟正黑體 */

	return (VS_SUCCESS);
}

/*
Function	:inDISP_TTF_SET
Date&Time	:2015/6/8 下午 2:50
Describe	:設定TTF字型及Style
*/
int inDISP_TTF_SetFont(int inLanguage, int inFontStyle)
{
	if (inLanguage == _DISP_CHINESE_)
	{
		CTOS_LCDTTFSelect((unsigned char *)_CHINESE_FONE_1_, inFontStyle); /* 微軟正黑體 */
		ginSetFont = _DISP_CHINESE_;									   /* 儲存目前的Font */
	}
	else if (inLanguage == _DISP_ENGLISH_)
	{
		CTOS_LCDTTFSelect((unsigned char *)_ENGLISH_FONE_1_, inFontStyle);
		ginSetFont = _DISP_ENGLISH_; /* 儲存目前的Font */
	}

	return (VS_SUCCESS);
}

/*
Function	:inDISP_ClearAll
Date&Time	:2015/6/7 下午 5:09
Describe	:清除整個螢幕
*/
int inDISP_ClearAll(void)
{
	// inDISP_PutGraphic(_CLEAR_ALL_SCREEN_, 0, _COORDINATE_Y_LINE_8_1_); /* 放置空白圖 */
	CTOS_LCDTClearDisplay();
	return (VS_SUCCESS);
}

/*
Function	:inDISP_Clear_Area
Date&Time	:2015/6/7 下午 6:41
Describe	:以8X16格式來指定位置清除螢幕
*/
int inDISP_Clear_Area(int inXL, int inYL, int inXR, int inYR, int inFoneSize)
{
	int i = 0;

	if (inFoneSize == _ENGLISH_FONT_8X16_)
		CTOS_LCDTSelectFontSize(_ENGLISH_FONT_8X16_);
	else if (inFoneSize == _ENGLISH_FONT_8X22_)
		CTOS_LCDTSelectFontSize(_ENGLISH_FONT_8X22_);

	for (i = inYL; i <= inYR; i++)
	{
		if (i == inYL)
			CTOS_LCDTGotoXY(inXL, i);
		else
			CTOS_LCDTGotoXY(1, i);

		CTOS_LCDTClear2EOL();
	}

	return (VS_SUCCESS);
}

/*
Function	:inDISP_Clear_Area
Date&Time	:2015/6/7 下午 6:41
Describe	:清除區塊，XY起始位置然後算Size
*/
int inDISP_Clear_Box(int inXL, int inYL, int inXSize, int inYSize)
{
	CTOS_LCDGSetBox(inXL, inYL, inXSize, inYSize, d_LCD_FILL_0);

	return (VS_SUCCESS);
}

/*
Function	:inDISP_Clear_Line
Date&Time	:2015/6/7 下午 5:09
Describe	:清除局部矩形螢幕
*/
int inDISP_Clear_Line(int inLineT, int inLineB)
{
	int i;

	if (d_OK != CTOS_LCDTSelectFontSize(_ENGLISH_FONT_8X16_))
	{
		inDISP_LogPrintfWithFlag("  ClearLine LCET Select *Error* [0x%04x] Line[%d]", d_OK, __LINE__);
	}

	for (i = inLineT; i <= inLineB; i++)
	{
		CTOS_LCDTGotoXY(1, i);
		CTOS_LCDTClear2EOL();
	}

	return (VS_SUCCESS);
}

/*
Function        :inDISP_Decide_FontSize
Date&Time       :2018/4/12 上午 10:57
Describe        :
*/
int inDISP_Decide_FontSize(int inFontSize, int inLanguage, unsigned short *usFontsize)
{
	if (inLanguage == _DISP_CHINESE_)
	{
		if (inFontSize == _FONTSIZE_8X16_)
			*usFontsize = _CHINESE_FONT_8X16_;
		else if (inFontSize == _FONTSIZE_8X22_)
			*usFontsize = _CHINESE_FONT_8X22_; /* 8 X 22 */
		else if (inFontSize == _FONTSIZE_8X44_)
			*usFontsize = _CHINESE_FONT_8X44_; /* 8 X 44 */
		else if (inFontSize == _FONTSIZE_12X19_)
			*usFontsize = _CHINESE_FONT_12X19_; /* 12 X 19 */
		else if (inFontSize == _FONTSIZE_16X16_)
			*usFontsize = _CHINESE_FONT_16X16_; /* 8 X 16 */
		else if (inFontSize == _FONTSIZE_16X22_)
			*usFontsize = _CHINESE_FONT_16X22_; /* 16 X 22 */
		else if (inFontSize == _FONTSIZE_16X44_)
			*usFontsize = _CHINESE_FONT_16X44_; /* 16 X 44 */
		else if (inFontSize == _FONTSIZE_32X22_)
			*usFontsize = _CHINESE_FONT_32X22_; /* 32 X 22 */
		else
			return (VS_ERROR);
	}
	else if (inLanguage == _DISP_ENGLISH_)
	{
		if (inFontSize == _FONTSIZE_8X16_)
			*usFontsize = _ENGLISH_FONT_8X16_; /* 8 X 16 */
		else if (inFontSize == _FONTSIZE_8X22_)
			*usFontsize = _ENGLISH_FONT_8X22_; /* 8 X 22 */
		else if (inFontSize == _FONTSIZE_8X44_)
			*usFontsize = _ENGLISH_FONT_8X44_; /* 8 X 44 */
		else if (inFontSize == _FONTSIZE_12X19_)
			*usFontsize = _ENGLISH_FONT_12X19_; /* 12 X 19 */
		else if (inFontSize == _FONTSIZE_16X16_)
			*usFontsize = _ENGLISH_FONT_16X16_; /* 16 X 22 */
		else if (inFontSize == _FONTSIZE_16X22_)
			*usFontsize = _ENGLISH_FONT_16X22_; /* 16 X 22 */
		else if (inFontSize == _FONTSIZE_16X44_)
			*usFontsize = _ENGLISH_FONT_16X44_; /* 16 X 44 */
		else if (inFontSize == _FONTSIZE_32X22_)
			*usFontsize = _ENGLISH_FONT_32X22_; /* 32 X 22 */
		else
			return (VS_ERROR);
	}
	else
	{
		if (inFontSize == _FONTSIZE_8X16_)
			*usFontsize = _CHINESE_FONT_8X16_; /* 8 X 16 */
		else if (inFontSize == _FONTSIZE_8X22_)
			*usFontsize = _CHINESE_FONT_8X22_; /* 8 X 22 */
		else if (inFontSize == _FONTSIZE_8X44_)
			*usFontsize = _CHINESE_FONT_8X44_; /* 8 X 44 */
		else if (inFontSize == _FONTSIZE_12X19_)
			*usFontsize = _CHINESE_FONT_12X19_; /* 12 X 19 */
		else if (inFontSize == _FONTSIZE_16X16_)
			*usFontsize = _CHINESE_FONT_16X16_; /* 8 X 16 */
		else if (inFontSize == _FONTSIZE_16X22_)
			*usFontsize = _CHINESE_FONT_16X22_; /* 16 X 22 */
		else if (inFontSize == _FONTSIZE_16X44_)
			*usFontsize = _CHINESE_FONT_16X44_; /* 16 X 44 */
		else if (inFontSize == _FONTSIZE_32X22_)
			*usFontsize = _CHINESE_FONT_32X22_; /* 32 X 22 */
		else
			return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inDISP_Select_FontSize
Date&Time       :2017/10/25 下午 4:47
Describe        :
*/
int inDISP_Select_FontSize(int inFontSize, int inLanguage)
{
	unsigned short usFontSize = 0;

	if (inDISP_Decide_FontSize(inFontSize, inLanguage, &usFontSize) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		CTOS_LCDTSelectFontSize(usFontSize);
	}

	return (VS_SUCCESS);
}

/*
Function	:inDISP_ChineseFont
Date&Time	:2015/6/7 下午 5:09
Describe	:顯示LCD中文字
*/
int inDISP_ChineseFont(char *szMessage, int inFontSize, int inLINE, int inAligned)
{
	int inRetVal = VS_SUCCESS;

	if (ginSetFont != _DISP_CHINESE_)
	{
		/* 判斷是否已經SetFont過_DISP_CHINESE_，如果沒有就要Set */
		inDISP_TTF_SetFont(_DISP_CHINESE_, _FONT_REGULAR_); /* 微軟正黑體 */
	}

	inRetVal = inDISP_Select_FontSize(inFontSize, _DISP_CHINESE_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	CTOS_LCDTPrintAligned(inLINE, (unsigned char *)szMessage, inAligned);
	return (VS_SUCCESS);
}

/*
Function	:inDISP_ChineseFont_Color
Date&Time	:2015/8/26 上午 9:58
Describe	:顯示LCD有顏色的中文字
*/
int inDISP_ChineseFont_Color(char *szMessage, int inFontSize, int inLINE, int inColor, int inAlign)
{
	int inRetVal = VS_SUCCESS;

	if (ginSetFont != _DISP_CHINESE_)
	{
		/* 判斷是否已經SetFont過_DISP_CHINESE_，如果沒有就要Set */
		inDISP_TTF_SetFont(_DISP_CHINESE_, _FONT_REGULAR_); /* 微軟正黑體 */
	}
	/* 將字體顏色換色 */
	CTOS_LCDForeGndColor(inColor);

	inRetVal = inDISP_Select_FontSize(inFontSize, _DISP_CHINESE_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	CTOS_LCDTPrintAligned(inLINE, (unsigned char *)szMessage, inAlign);

	/* 將字體顏色換回黑色 */
	CTOS_LCDForeGndColor(0x00000000);

	return (VS_SUCCESS);
}

int inDISP_ChineseFont_Point_Color(char *szMessage, int inFontSize, int inLINE, int inForeColor, int inBackColor, int inX)
{
	int inRetVal = VS_SUCCESS;

	if (ginSetFont != _DISP_CHINESE_)
	{
		/* 判斷是否已經SetFont過_DISP_CHINESE_，如果沒有就要Set */
		inDISP_TTF_SetFont(_DISP_CHINESE_, _FONT_REGULAR_); /* 微軟正黑體 */
	}

	/* 將字體顏色換色 */
	CTOS_LCDForeGndColor(inForeColor);
	/* 將背景顏色換色 */
	CTOS_LCDBackGndColor(inBackColor);

	inRetVal = inDISP_Select_FontSize(inFontSize, _DISP_CHINESE_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = CTOS_LCDTPrintXY(inX, inLINE, (unsigned char *)szMessage);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			char szDebugMsg[100 + 1];

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	/* 將字體顏色換回黑色 */
	CTOS_LCDForeGndColor(0x00000000);
	/* 將背景顏色換回白色 */
	CTOS_LCDBackGndColor(0x00FFFFFF);

	return (VS_SUCCESS);
}

/*
Function        :inDISP_ChineseFont_Point_Color_By_Graphic_Mode
Date&Time       :2018/4/11 上午 9:20
Describe        :如果用Text Mode會有中文字和英文字對不齊的問題，這時候可以改用Graphic_Mode
*/
int inDISP_ChineseFont_Point_Color_By_Graphic_Mode(char *szMessage, int inFontSize, int inForeColor, int inBackColor, int inX, int inY, unsigned char uszReverse)
{
	int inRetVal = VS_SUCCESS;
	unsigned short usFontSize = 0;

	if (ginSetFont != _DISP_CHINESE_)
	{
		/* 判斷是否已經SetFont過_DISP_CHINESE_，如果沒有就要Set */
		inDISP_TTF_SetFont(_DISP_CHINESE_, _FONT_REGULAR_); /* 微軟正黑體 */
	}

	/* 將字體顏色換色 */
	CTOS_LCDForeGndColor(inForeColor);
	/* 將背景顏色換色 */
	CTOS_LCDBackGndColor(inBackColor);

	inDISP_Decide_FontSize(inFontSize, _DISP_CHINESE_, &usFontSize);

	inRetVal = CTOS_LCDGTextOut(inX, inY, (unsigned char *)szMessage, usFontSize, uszReverse);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			char szDebugMsg[100 + 1];

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "0x%04X", inRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	/* 將字體顏色換回黑色 */
	CTOS_LCDForeGndColor(0x00000000);
	/* 將背景顏色換回白色 */
	CTOS_LCDBackGndColor(0x00FFFFFF);

	return (VS_SUCCESS);
}

/*
Function	:inDISP_EnglishFont
Date&Time	:2015/6/7 下午 5:09
Describe	:顯示LCD英數字
*/
int inDISP_EnglishFont(char *szMessage, int inFontSize, int inLINE, int inAlign)
{
	int inRetVal = VS_SUCCESS;

	if (ginSetFont != _DISP_ENGLISH_)
	{
		/* 判斷是否已經SetFont過_DISP_ENGLISH_，如果沒有就要Set */
		inDISP_TTF_SetFont(_DISP_ENGLISH_, _FONT_REGULAR_);
	}

	inRetVal = inDISP_Select_FontSize(inFontSize, _DISP_ENGLISH_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	CTOS_LCDTPrintAligned(inLINE, (unsigned char *)szMessage, inAlign);

	return (VS_SUCCESS);
}

/*
Function        :inDISP_EnglishFont
Date&Time       :2015/8/28 下午 9:45
Describe        :顯示LCD彩色英數字
*/
int inDISP_EnglishFont_Color(char *szMessage, int inFontSize, int inLINE, int inColor, int inAlign)
{
	int inRetVal = VS_SUCCESS;

	if (ginSetFont != _DISP_ENGLISH_)
	{
		/* 判斷是否已經SetFont過_DISP_CHINESE_，如果沒有就要Set */
		inDISP_TTF_SetFont(_DISP_ENGLISH_, _FONT_REGULAR_); /* 微軟正黑體 */
	}
	/* 將字體顏色換色 */
	CTOS_LCDForeGndColor(inColor);

	inRetVal = inDISP_Select_FontSize(inFontSize, _DISP_ENGLISH_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	CTOS_LCDTPrintAligned(inLINE, (unsigned char *)szMessage, inAlign);

	/* 將字體顏色換回黑色 */
	CTOS_LCDForeGndColor(0x00000000);

	return (VS_SUCCESS);
}

/*
Function        :inDISP_EnglishFont_Point_Color
Date&Time       :2016/1/17 下午 9:00
Describe        :顯示LCD彩色英數字可以為設定顯示起始點
*/
int inDISP_EnglishFont_Point_Color(char *szMessage, int inFontSize, int inLINE, int inForeColor, int inBackColor, int inX)
{
	int inRetVal = VS_SUCCESS;

	if (ginSetFont != _DISP_ENGLISH_)
	{
		/* 判斷是否已經SetFont過_DISP_CHINESE_，如果沒有就要Set */
		inDISP_TTF_SetFont(_DISP_ENGLISH_, _FONT_REGULAR_); /* 微軟正黑體 */
	}

	/* 將字體顏色換色 */
	CTOS_LCDForeGndColor(inForeColor);
	/* 將背景顏色換色 */
	CTOS_LCDBackGndColor(inBackColor);

	inRetVal = inDISP_Select_FontSize(inFontSize, _DISP_ENGLISH_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	CTOS_LCDTPrintXY(inX, inLINE, (unsigned char *)szMessage);

	/* 將字體顏色換回黑色 */
	CTOS_LCDForeGndColor(0x00000000);
	/* 將背景顏色換回白色 */
	CTOS_LCDBackGndColor(0x00FFFFFF);

	return (VS_SUCCESS);
}

/*
Function	:inPutGraphic
Date&Time	:2015/6/7 下午 5:09
Describe	:顯示LCD BMP圖檔
*/
int inDISP_PutGraphic(char *szFileName, int inX, int inY)
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal = 0;

	usRetVal = CTOS_LCDGShowBMPPic(inX, inY, (BYTE *)szFileName);
	if (usRetVal == d_OK)
	{
		//                if (ginDebug == VS_TRUE)
		//		{
		//                    inDISP_LogPrintf(szFileName);
		//                }
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf(szFileName);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_LCDGShowBMPPic Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function	:inDISP_Enter8x16
Date&Time	:2015/6/22 下午 8:11
Describe	:輸入數字
*/
int inDISP_Enter8x16(DISPLAY_OBJECT *srDispObj)
{
	int inColor;
	int inFinalTimeOut, inY_16X22, inY_12X19, inY_16X22_2;
	int inPart1Len = 0, inPart2Len = 0;
	char szTemplate[_DISP_MSG_SIZE_ + 1], szMaskOutput[_DISP_MSG_SIZE_ + 1];
	char szTemplate2[_DISP_MSG_SIZE_ + 1], szMaskOutput2[_DISP_MSG_SIZE_ + 1];
	char szTemp[_DISP_MSG_SIZE_ + 1], szTemp2[_DISP_MSG_SIZE_ + 1];
	unsigned char uszkey;

	if (srDispObj->inMenuKeyIn > 0)
	{
		/* 將輸入第一碼先存起來 */
		sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", srDispObj->inMenuKeyIn);
		srDispObj->inOutputLen++;
	}

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	if (srDispObj->inTimeout > 0)
	{
		inFinalTimeOut = srDispObj->inTimeout;
	}
	else
	{
		inFinalTimeOut = _EDC_TIMEOUT_;
	}

	inColor = srDispObj->inColor;

	while (1)
	{
		uszkey = -1;

		if (srDispObj->inMenuKeyIn > 0)
		{
			/* 如果為MenuKeyIn，第一個數字要顯示 */
			uszkey = _MENUKEYIN_EVENT_;
		}
		else
		{
			uszkey = uszKBD_GetKey(inFinalTimeOut);
		}

		switch (uszkey)
		{
		case _KEY_CANCEL_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_USER_CANCEL);
		case _KEY_TIMEOUT_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_TIMEOUT);
		case _KEY_ENTER_:
			/* 先確認該部份是否可輸入0 和 ByPass */
			if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				return (srDispObj->inOutputLen);
			}
			/* 不能ByPass但可以輸入0 */
			else if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass == VS_TRUE)
			{
				if (srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0但可以ByPass */
			else if (srDispObj->inCanNotZero == VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0也不能ByPass */
			else
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L || srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
		case _KEY_CLEAR_:
			if (srDispObj->inOutputLen > 0)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;
				break;
			}
			else
				continue;
		case _KEY_0_:
		case _KEY_1_:
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			/* 若超過最大長度時長嗶一聲 */
			if (srDispObj->inOutputLen >= srDispObj->inMaxLen)
			{
				inDISP_BEEP(1, 1);
				continue;
			}

			sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", uszkey);
			srDispObj->inOutputLen++;
			break;
		case _MENUKEYIN_EVENT_:
			/* 將inMenuKeyIn初始化 */
			srDispObj->inMenuKeyIn = -1;
			break;
		default:
			continue;
		}

		/* 如果長度大於16，將8X16轉8X23顯示 */
		if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) <= 19 && srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 16)
		{
			/* 將8行模式顯示轉換成10行模式 */
			inY_12X19 = (srDispObj->inY * 3) / 2;
			/* 設定螢幕字型大小 */
			srDispObj->inFoneSize = _ENGLISH_FONT_8X22_;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcat(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_12X19_, inY_12X19, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_12X19_, inY_12X19, inColor, _DISP_RIGHT_);
			}
		}
		else if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) <= 22 && srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 19)
		{
			/* 將8行模式顯示轉換成16行模式 */
			inY_16X22 = srDispObj->inY * 2 - 1;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_RIGHT_);
			}
		}
		else if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 22)
		{
			/* 將8行模式顯示轉換成16行模式 */
			inY_16X22 = srDispObj->inY * 2 - 1;
			inY_16X22_2 = srDispObj->inY * 2;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			inPart1Len = 22 - strlen(srDispObj->szPromptMsg);
			inPart2Len = srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) - 22;

			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{

					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', inPart1Len);

					memset(szMaskOutput2, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput2, '*', inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
				}
				else
				{
					memset(szTemp, 0x00, sizeof(szTemp));
					memcpy(szTemp, srDispObj->szOutput, inPart1Len);

					memset(szTemp2, 0x00, sizeof(szTemp2));
					memcpy(szTemp2, &srDispObj->szOutput[inPart1Len], inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
					else
					{
						strcpy(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_LEFT_);
				inDISP_EnglishFont_Color(szTemplate2, _FONTSIZE_16X22_, inY_16X22_2, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', inPart1Len);

					memset(szMaskOutput2, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput2, '*', inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
				}
				else
				{
					memset(szTemp, 0x00, sizeof(szTemp));
					memcpy(szTemp, srDispObj->szOutput, inPart1Len);

					memset(szTemp2, 0x00, sizeof(szTemp2));
					memcpy(szTemp2, &srDispObj->szOutput[inPart1Len], inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
					else
					{
						strcpy(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_RIGHT_);
				inDISP_EnglishFont_Color(szTemplate2, _FONTSIZE_16X22_, inY_16X22_2, inColor, _DISP_RIGHT_);
			}
		}
		else
		{
			/* 設定螢幕字型大小 */
			srDispObj->inFoneSize = _ENGLISH_FONT_8X16_;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcat(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_RIGHT_);
			}
		}
	}
}

/*
Function	:inDISP_Enter8x16_NumDot
Date&Time	:2016/1/6 上午 10:20
Describe	:輸入數字和點 主要是輸入IP用
*/
int inDISP_Enter8x16_NumDot(DISPLAY_OBJECT *srDispObj)
{
	int inColor;
	int inFinalTimeOut, inY_16X22, inY_12X19, inY_16X22_2;
	int inPart1Len = 0, inPart2Len = 0;
	char szTemplate[_DISP_MSG_SIZE_ + 1], szMaskOutput[_DISP_MSG_SIZE_ + 1];
	char szTemplate2[_DISP_MSG_SIZE_ + 1], szMaskOutput2[_DISP_MSG_SIZE_ + 1];
	char szTemp[_DISP_MSG_SIZE_ + 1], szTemp2[_DISP_MSG_SIZE_ + 1];
	unsigned char uszkey;

	if (srDispObj->inMenuKeyIn > 0)
	{
		/* 將輸入第一碼先存起來 */
		sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", srDispObj->inMenuKeyIn);
		srDispObj->inOutputLen++;
	}

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	if (srDispObj->inTimeout > 0)
	{
		inFinalTimeOut = srDispObj->inTimeout;
	}
	else
	{
		inFinalTimeOut = _EDC_TIMEOUT_;
	}

	inColor = srDispObj->inColor;

	while (1)
	{
		uszkey = -1;

		if (srDispObj->inMenuKeyIn > 0)
		{
			/* 如果為MenuKeyIn，第一個數字要顯示 */
			uszkey = _MENUKEYIN_EVENT_;
		}
		else
		{
			uszkey = uszKBD_GetKey(inFinalTimeOut);
		}

		switch (uszkey)
		{
		case _KEY_CANCEL_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_USER_CANCEL);
		case _KEY_TIMEOUT_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_TIMEOUT);
		case _KEY_ENTER_:
			/* 先確認該部份是否可輸入0 和 ByPass */
			if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				return (srDispObj->inOutputLen);
			}
			/* 不能ByPass但可以輸入0 */
			else if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass == VS_TRUE)
			{
				if (srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0但可以ByPass */
			else if (srDispObj->inCanNotZero == VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0也不能ByPass */
			else
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L || srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
		case _KEY_CLEAR_:
			if (srDispObj->inOutputLen > 0)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;
				break;
			}
			else
				continue;
		case _KEY_0_:
		case _KEY_1_:
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			/* 若超過最大長度時長嗶一聲 */
			if (srDispObj->inOutputLen >= srDispObj->inMaxLen)
			{
				inDISP_BEEP(1, 1);
				continue;
			}

			sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", uszkey);
			srDispObj->inOutputLen++;
			break;
		case _KEY_DOT_:
			if (srDispObj->inOutputLen >= srDispObj->inMaxLen)
				continue;

			sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], ".");
			srDispObj->inOutputLen++;
			break;
		case _MENUKEYIN_EVENT_:
			/* 將inMenuKeyIn初始化 */
			srDispObj->inMenuKeyIn = -1;
			break;
		default:
			continue;
		}

		/* 如果長度大於16，將8X16轉8X23顯示 */
		if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) <= 19 && srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 16)
		{
			/* 將8行模式顯示轉換成10行模式 */
			inY_12X19 = (srDispObj->inY * 3) / 2;
			/* 設定螢幕字型大小 */
			srDispObj->inFoneSize = _ENGLISH_FONT_8X22_;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcat(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_12X19_, inY_12X19, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_12X19_, inY_12X19, inColor, _DISP_RIGHT_);
			}
		}
		else if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) <= 22 && srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 19)
		{
			/* 將8行模式顯示轉換成16行模式 */
			inY_16X22 = srDispObj->inY * 2 - 1;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_RIGHT_);
			}
		}
		else if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 22)
		{
			/* 將8行模式顯示轉換成16行模式 */
			inY_16X22 = srDispObj->inY * 2 - 1;
			inY_16X22_2 = srDispObj->inY * 2;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			inPart1Len = 22 - strlen(srDispObj->szPromptMsg);
			inPart2Len = srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) - 22;

			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{

					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', inPart1Len);

					memset(szMaskOutput2, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput2, '*', inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
				}
				else
				{
					memset(szTemp, 0x00, sizeof(szTemp));
					memcpy(szTemp, srDispObj->szOutput, inPart1Len);

					memset(szTemp2, 0x00, sizeof(szTemp2));
					memcpy(szTemp2, &srDispObj->szOutput[inPart1Len], inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
					else
					{
						strcpy(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_LEFT_);
				inDISP_EnglishFont_Color(szTemplate2, _FONTSIZE_16X22_, inY_16X22_2, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', inPart1Len);

					memset(szMaskOutput2, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput2, '*', inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
				}
				else
				{
					memset(szTemp, 0x00, sizeof(szTemp));
					memcpy(szTemp, srDispObj->szOutput, inPart1Len);

					memset(szTemp2, 0x00, sizeof(szTemp2));
					memcpy(szTemp2, &srDispObj->szOutput[inPart1Len], inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
					else
					{
						strcpy(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_RIGHT_);
				inDISP_EnglishFont_Color(szTemplate2, _FONTSIZE_16X22_, inY_16X22_2, inColor, _DISP_RIGHT_);
			}
		}
		else
		{
			/* 設定螢幕字型大小 */
			srDispObj->inFoneSize = _ENGLISH_FONT_8X16_;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcat(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_RIGHT_);
			}
		}
	}
}

/*
Function	:inDISP_Enter8x16_Character
Date&Time	:2015/7/5 下午 7:49
Describe	:輸入字元包含英數字，建議使用inDISP_Enter8x16_Character_Mask，相當於inDISP_Enter8x16_Character_Mask但不把inMask設為On
*/
int inDISP_Enter8x16_Character(DISPLAY_OBJECT *srDispObj)
{
	int inColor;
	int inNumberKey = -1, inAlphaKeyIndex = 0;
	int inFinalTimeOut, inY_16X22, inY_12X19, inY_16X22_2;
	int inPart1Len = 0, inPart2Len = 0;
	char szTemplate[_DISP_MSG_SIZE_ + 1], szMaskOutput[_DISP_MSG_SIZE_ + 1];
	char szTemplate2[_DISP_MSG_SIZE_ + 1], szMaskOutput2[_DISP_MSG_SIZE_ + 1];
	char szTemp[_DISP_MSG_SIZE_ + 1], szTemp2[_DISP_MSG_SIZE_ + 1];
	char CharSet[10][15] = {{"0.,@* -"}, {"1QZqz"}, {"2ABCabc"}, {"3DEFdef"}, {"4GHIghi"}, {"5JKLjkl"}, {"6MNOmno"}, {"7PRSprs"}, {"8TUVtuv"}, {"9WXYwxy"}};
	unsigned char uszkey = 0x00;
	unsigned char uszFindAlpha = VS_FALSE;

	if (srDispObj->inMenuKeyIn > 0)
	{
		/* 將輸入第一碼先存起來 */
		sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", srDispObj->inMenuKeyIn);
		srDispObj->inOutputLen++;
	}

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	if (srDispObj->inTimeout > 0)
	{
		inFinalTimeOut = srDispObj->inTimeout;
	}
	else
	{
		inFinalTimeOut = _EDC_TIMEOUT_;
	}

	inColor = srDispObj->inColor;

	while (1)
	{
		uszkey = -1;

		if (srDispObj->inMenuKeyIn > 0)
		{
			/* 如果為MenuKeyIn，第一個數字要顯示 */
			uszkey = _MENUKEYIN_EVENT_;
		}
		else
		{
			uszkey = uszKBD_GetKey(inFinalTimeOut);
		}

		switch (uszkey)
		{
		case _KEY_CANCEL_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_USER_CANCEL);
		case _KEY_TIMEOUT_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_TIMEOUT);
		case _KEY_ENTER_:
			/* 先確認該部份是否可輸入0 和 ByPass */
			if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				return (srDispObj->inOutputLen);
			}
			/* 不能ByPass但可以輸入0 */
			else if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass == VS_TRUE)
			{
				if (srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0但可以ByPass */
			else if (srDispObj->inCanNotZero == VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0也不能ByPass */
			else
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L || srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
		case _KEY_CLEAR_:
			if (srDispObj->inOutputLen > 1)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				szMaskOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;

				uszFindAlpha = VS_FALSE;
				for (inNumberKey = 0; inNumberKey < 10; inNumberKey++)
				{
					for (inAlphaKeyIndex = 0; inAlphaKeyIndex < 15; inAlphaKeyIndex++)
					{
						if (CharSet[inNumberKey][inAlphaKeyIndex] == srDispObj->szOutput[srDispObj->inOutputLen - 1])
						{
							uszFindAlpha = VS_TRUE;
						}

						/* 找到就一路break出去 */
						if (uszFindAlpha == VS_TRUE)
						{
							break;
						}
					}

					/* 找到就一路break出去 */
					if (uszFindAlpha == VS_TRUE)
					{
						break;
					}
				}

				/* 找到就一路break出去 */
				if (uszFindAlpha == VS_TRUE)
				{
				}
				else
				{
					inAlphaKeyIndex = 0;
					inNumberKey = -1;
				}
				break;
			}
			else if (srDispObj->inOutputLen == 1)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				szMaskOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;

				inAlphaKeyIndex = 0;
				inNumberKey = -1;
				break;
			}
			else
			{
				inAlphaKeyIndex = 0;
				inNumberKey = -1;
				continue;
			}
		case _KEY_0_:
		case _KEY_1_:
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			/* 若超過最大長度時長嗶一聲 */
			if (srDispObj->inOutputLen >= srDispObj->inMaxLen)
			{
				inDISP_BEEP(1, 1);
				continue;
			}

			sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", uszkey);
			srDispObj->inOutputLen++;

			/* 將當下的Array存起來，以便切換Alpha鍵 */
			inNumberKey = uszkey - 48;
			/* 回到當初的index */
			inAlphaKeyIndex = 0;
			break;
		case _KEY_ALPHA_:
			if (inNumberKey == -1)
				continue;

			inAlphaKeyIndex++;

			switch (inNumberKey)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				if (inAlphaKeyIndex >= strlen(CharSet[inNumberKey]))
					inAlphaKeyIndex = 0;
				break;
			}

			/* 至Array中抓取對應位置的符號再塞到srDispObj->szOutput */
			memcpy(&srDispObj->szOutput[srDispObj->inOutputLen - 1], &CharSet[inNumberKey][inAlphaKeyIndex], 1);

			break;
		case _MENUKEYIN_EVENT_:
			/* 將inMenuKeyIn初始化 */
			srDispObj->inMenuKeyIn = -1;
			break;
		default:
			continue;
		}

		/* 如果長度大於16，將8X16轉8X23顯示 */
		if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) <= 19 && srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 16)
		{
			/* 將8行模式顯示轉換成10行模式 */
			inY_12X19 = (srDispObj->inY * 3) / 2;
			/* 設定螢幕字型大小 */
			srDispObj->inFoneSize = _ENGLISH_FONT_8X22_;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcat(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_12X19_, inY_12X19, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_12X19_, inY_12X19, inColor, _DISP_RIGHT_);
			}
		}
		else if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) <= 22 && srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 19)
		{
			/* 將8行模式顯示轉換成16行模式 */
			inY_16X22 = srDispObj->inY * 2 - 1;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_RIGHT_);
			}
		}
		else if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 22)
		{
			/* 將8行模式顯示轉換成16行模式 */
			inY_16X22 = srDispObj->inY * 2 - 1;
			inY_16X22_2 = srDispObj->inY * 2;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			inPart1Len = 22 - strlen(srDispObj->szPromptMsg);
			inPart2Len = srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) - 22;

			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{

					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', inPart1Len);

					memset(szMaskOutput2, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput2, '*', inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
				}
				else
				{
					memset(szTemp, 0x00, sizeof(szTemp));
					memcpy(szTemp, srDispObj->szOutput, inPart1Len);

					memset(szTemp2, 0x00, sizeof(szTemp2));
					memcpy(szTemp2, &srDispObj->szOutput[inPart1Len], inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
					else
					{
						strcpy(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_LEFT_);
				inDISP_EnglishFont_Color(szTemplate2, _FONTSIZE_16X22_, inY_16X22_2, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', inPart1Len);

					memset(szMaskOutput2, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput2, '*', inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
				}
				else
				{
					memset(szTemp, 0x00, sizeof(szTemp));
					memcpy(szTemp, srDispObj->szOutput, inPart1Len);

					memset(szTemp2, 0x00, sizeof(szTemp2));
					memcpy(szTemp2, &srDispObj->szOutput[inPart1Len], inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
					else
					{
						strcpy(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_RIGHT_);
				inDISP_EnglishFont_Color(szTemplate2, _FONTSIZE_16X22_, inY_16X22_2, inColor, _DISP_RIGHT_);
			}
		}
		else
		{
			/* 設定螢幕字型大小 */
			srDispObj->inFoneSize = _ENGLISH_FONT_8X16_;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcat(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_RIGHT_);
			}
		}
	}
}

/*
Function	:inDISP_Enter8x16_Character_Mask
Date&Time	:2015/7/5 下午 7:49
Describe	:輸入字元包含Mask
*/
int inDISP_Enter8x16_Character_Mask(DISPLAY_OBJECT *srDispObj)
{
	int inChoice = 0;
	int inColor = _COLOR_RED_;
	int inNumberKey = -1, inAlphaKeyIndex = 0;
	int inFinalTimeOut, inY_16X22, inY_12X19, inY_16X22_2;
	int inPart1Len = 0, inPart2Len = 0;
	char szTemplate[_DISP_MSG_SIZE_ + 1], szMaskOutput[_DISP_MSG_SIZE_ + 1];
	char szTemplate2[_DISP_MSG_SIZE_ + 1], szMaskOutput2[_DISP_MSG_SIZE_ + 1];
	char szTemp[_DISP_MSG_SIZE_ + 1], szTemp2[_DISP_MSG_SIZE_ + 1];
	char CharSet[10][15] = {{"0.,@* -:/_"}, {"1QZqz"}, {"2ABCabc"}, {"3DEFdef"}, {"4GHIghi"}, {"5JKLjkl"}, {"6MNOmno"}, {"7PRSprs"}, {"8TUVtuv"}, {"9WXYwxy"}};
	unsigned char uszkey = 0x00;
	unsigned char uszFindAlpha = VS_FALSE;

	if (srDispObj->inMenuKeyIn > 0)
	{
		/* 將輸入第一碼先存起來 */
		sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", srDispObj->inMenuKeyIn);
		srDispObj->inOutputLen++;
	}

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	if (srDispObj->inTimeout > 0)
	{
		inFinalTimeOut = srDispObj->inTimeout;
	}
	else
	{
		inFinalTimeOut = _EDC_TIMEOUT_;
	}

	inColor = srDispObj->inColor;

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, inFinalTimeOut);
	while (1)
	{
		uszkey = -1;

		if (srDispObj->inMenuKeyIn > 0)
		{
			/* 如果為MenuKeyIn，第一個數字要顯示 */
			uszkey = _MENUKEYIN_EVENT_;
		}
		else
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(srDispObj->inTouchSensorFunc);
			uszkey = uszKBD_Key();
		}

		if (inChoice == _Touch_OX_LINE8_8_ENTER_BUTTON_)
		{
			uszkey = _KEY_ENTER_;
		}
		else if (inChoice == _Touch_OX_LINE8_8_CANCEL_BUTTON_)
		{
			uszkey = _KEY_CANCEL_;
		}

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszkey = _KEY_TIMEOUT_;
		}

		switch (uszkey)
		{
		case _KEY_CANCEL_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_USER_CANCEL);
		case _KEY_TIMEOUT_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_TIMEOUT);
		case _KEY_ENTER_:
			/* 先確認該部份是否可輸入0 和 ByPass */
			if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				return (srDispObj->inOutputLen);
			}
			/* 不能ByPass但可以輸入0 */
			else if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass == VS_TRUE)
			{
				if (srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0但可以ByPass */
			else if (srDispObj->inCanNotZero == VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0也不能ByPass */
			else
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L || srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
		case _KEY_CLEAR_:
			if (srDispObj->inOutputLen > 1)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				szMaskOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;

				uszFindAlpha = VS_FALSE;
				for (inNumberKey = 0; inNumberKey < 10; inNumberKey++)
				{
					for (inAlphaKeyIndex = 0; inAlphaKeyIndex < 15; inAlphaKeyIndex++)
					{
						if (CharSet[inNumberKey][inAlphaKeyIndex] == srDispObj->szOutput[srDispObj->inOutputLen - 1])
						{
							uszFindAlpha = VS_TRUE;
						}

						/* 找到就一路break出去 */
						if (uszFindAlpha == VS_TRUE)
						{
							break;
						}
					}

					/* 找到就一路break出去 */
					if (uszFindAlpha == VS_TRUE)
					{
						break;
					}
				}

				/* 找到就一路break出去 */
				if (uszFindAlpha == VS_TRUE)
				{
				}
				else
				{
					inAlphaKeyIndex = 0;
					inNumberKey = -1;
				}
				break;
			}
			else if (srDispObj->inOutputLen == 1)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				szMaskOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;

				inAlphaKeyIndex = 0;
				inNumberKey = -1;
				break;
			}
			else
			{
				inAlphaKeyIndex = 0;
				inNumberKey = -1;
				continue;
			}
		case _KEY_0_:
		case _KEY_1_:
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			/* 若超過最大長度時長嗶一聲 */
			if (srDispObj->inOutputLen >= srDispObj->inMaxLen)
			{
				inDISP_BEEP(1, 1);
				continue;
			}

			sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", uszkey);
			srDispObj->inOutputLen++;

			/* 將當下的Array存起來，以便切換Alpha鍵 */
			inNumberKey = uszkey - 48;
			/* 回到當初的index */
			inAlphaKeyIndex = 0;
			break;
		case _KEY_ALPHA_:
			if (inNumberKey == -1)
				continue;

			inAlphaKeyIndex++;

			switch (inNumberKey)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				if (inAlphaKeyIndex >= strlen(CharSet[inNumberKey]))
					inAlphaKeyIndex = 0;
				break;
			}

			/* 至Array中抓取對應位置的符號再塞到srDispObj->szOutput */
			memcpy(&srDispObj->szOutput[srDispObj->inOutputLen - 1], &CharSet[inNumberKey][inAlphaKeyIndex], 1);

			break;
		case _MENUKEYIN_EVENT_:
			/* 將inMenuKeyIn初始化 */
			srDispObj->inMenuKeyIn = -1;
			break;
		default:
			continue;
		}

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, inFinalTimeOut);

		/* 如果長度大於16，將8X16轉8X23顯示 */
		if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) <= 19 && srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 16)
		{
			/* 將8行模式顯示轉換成10行模式 */
			inY_12X19 = (srDispObj->inY * 3) / 2;
			/* 設定螢幕字型大小 */
			srDispObj->inFoneSize = _ENGLISH_FONT_8X22_;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcat(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_12X19_, inY_12X19, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_12X19_, inY_12X19, inColor, _DISP_RIGHT_);
			}
		}
		else if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) <= 22 && srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 19)
		{
			/* 將8行模式顯示轉換成16行模式 */
			inY_16X22 = srDispObj->inY * 2 - 1;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_RIGHT_);
			}
		}
		else if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 22)
		{
			/* 將8行模式顯示轉換成16行模式 */
			inY_16X22 = srDispObj->inY * 2 - 1;
			inY_16X22_2 = srDispObj->inY * 2;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			inPart1Len = 22 - strlen(srDispObj->szPromptMsg);
			inPart2Len = srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) - 22;

			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{

					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', inPart1Len);

					memset(szMaskOutput2, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput2, '*', inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
				}
				else
				{
					memset(szTemp, 0x00, sizeof(szTemp));
					memcpy(szTemp, srDispObj->szOutput, inPart1Len);

					memset(szTemp2, 0x00, sizeof(szTemp2));
					memcpy(szTemp2, &srDispObj->szOutput[inPart1Len], inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
					else
					{
						strcpy(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_LEFT_);
				inDISP_EnglishFont_Color(szTemplate2, _FONTSIZE_16X22_, inY_16X22_2, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', inPart1Len);

					memset(szMaskOutput2, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput2, '*', inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
				}
				else
				{
					memset(szTemp, 0x00, sizeof(szTemp));
					memcpy(szTemp, srDispObj->szOutput, inPart1Len);

					memset(szTemp2, 0x00, sizeof(szTemp2));
					memcpy(szTemp2, &srDispObj->szOutput[inPart1Len], inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
					else
					{
						strcpy(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_RIGHT_);
				inDISP_EnglishFont_Color(szTemplate2, _FONTSIZE_16X22_, inY_16X22_2, inColor, _DISP_RIGHT_);
			}
		}
		else
		{
			/* 設定螢幕字型大小 */
			srDispObj->inFoneSize = _ENGLISH_FONT_8X16_;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcat(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}
				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_RIGHT_);
			}
		}
	}
}

/*
Function	:inDISP_Enter8x16_Character_Mask_And_DisTouch
Date&Time	:2017/8/24 下午 5:39
Describe	:再加入觸控的界面
*/
int inDISP_Enter8x16_Character_Mask_And_DisTouch(DISPLAY_OBJECT *srDispObj)
{
	int inChoice = 0;
	int inColor = _COLOR_RED_;
	int inNumberKey = -1, inAlphaKeyIndex = 1;
	int inFinalTimeOut, inY_16X22, inY_12X19, inY_16X22_2;
	int inPart1Len = 0, inPart2Len = 0;
	char szTemplate[_DISP_MSG_SIZE_ + 1], szMaskOutput[_DISP_MSG_SIZE_ + 1];
	char szTemplate2[_DISP_MSG_SIZE_ + 1], szMaskOutput2[_DISP_MSG_SIZE_ + 1];
	char szTemp[_DISP_MSG_SIZE_ + 1], szTemp2[_DISP_MSG_SIZE_ + 1];
	char CharSet[10][15] = {{"0.,@* -:/_"}, {"1QZqz"}, {"2ABCabc"}, {"3DEFdef"}, {"4GHIghi"}, {"5JKLjkl"}, {"6MNOmno"}, {"7PRSprs"}, {"8TUVtuv"}, {"9WXYwxy"}};
	unsigned char uszkey = 0x00;
	unsigned char uszFindAlpha = VS_FALSE;

	memset(szMaskOutput, 0x00, sizeof(szMaskOutput));

	if (srDispObj->inMenuKeyIn > 0)
	{
		/* 將輸入第一碼先存起來 */
		sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", srDispObj->inMenuKeyIn);
		srDispObj->inOutputLen++;
	}

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	if (srDispObj->inTimeout > 0)
	{
		inFinalTimeOut = srDispObj->inTimeout;
	}
	else
	{
		inFinalTimeOut = _EDC_TIMEOUT_;
	}

	inColor = srDispObj->inColor;

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, inFinalTimeOut);
	while (1)
	{
		uszkey = -1;

		if (srDispObj->inMenuKeyIn > 0)
		{
			/* 如果為MenuKeyIn，第一個數字要顯示 */
			uszkey = _MENUKEYIN_EVENT_;
		}
		else
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(srDispObj->inTouchSensorFunc);
			uszkey = uszKBD_Key();
		}

		if (inChoice == _CUPLogOn_Touch_KEY_2_)
		{
			uszkey = _KEY_ENTER_;
		}

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszkey = _KEY_TIMEOUT_;
		}

		switch (uszkey)
		{
		case _KEY_CANCEL_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_USER_CANCEL);
		case _KEY_TIMEOUT_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_TIMEOUT);
		case _KEY_ENTER_:
			/* 先確認該部份是否可輸入0 和 ByPass */
			if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				return (srDispObj->inOutputLen);
			}
			/* 不能ByPass但可以輸入0 */
			else if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass == VS_TRUE)
			{
				if (srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0但可以ByPass */
			else if (srDispObj->inCanNotZero == VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0也不能ByPass */
			else
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L || srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
		case _KEY_CLEAR_:
			if (srDispObj->inOutputLen > 1)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				szMaskOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;

				uszFindAlpha = VS_FALSE;
				for (inNumberKey = 0; inNumberKey < 10; inNumberKey++)
				{
					for (inAlphaKeyIndex = 0; inAlphaKeyIndex < 15; inAlphaKeyIndex++)
					{
						if (CharSet[inNumberKey][inAlphaKeyIndex] == srDispObj->szOutput[srDispObj->inOutputLen - 1])
						{
							uszFindAlpha = VS_TRUE;
						}

						/* 找到就一路break出去 */
						if (uszFindAlpha == VS_TRUE)
						{
							break;
						}
					}

					/* 找到就一路break出去 */
					if (uszFindAlpha == VS_TRUE)
					{
						break;
					}
				}

				/* 找到就一路break出去 */
				if (uszFindAlpha == VS_TRUE)
				{
				}
				else
				{
					inAlphaKeyIndex = 0;
					inNumberKey = -1;
				}
				break;
			}
			else if (srDispObj->inOutputLen == 1)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				szMaskOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;

				inAlphaKeyIndex = 0;
				inNumberKey = -1;
				break;
			}
			else
			{
				inAlphaKeyIndex = 0;
				inNumberKey = -1;
				continue;
			}
		case _KEY_0_:
		case _KEY_1_:
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			/* 若超過最大長度時長嗶一聲 */
			if (srDispObj->inOutputLen >= srDispObj->inMaxLen)
			{
				inDISP_BEEP(1, 1);
				continue;
			}

			sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", uszkey);
			srDispObj->inOutputLen++;

			/* 將當下的Array存起來，以便切換Alpha鍵 */
			inNumberKey = uszkey - 48;
			/* 回到當初的index */
			inAlphaKeyIndex = 0;
			break;
		case _KEY_ALPHA_:
			if (inNumberKey == -1)
				continue;

			inAlphaKeyIndex++;

			switch (inNumberKey)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				if (inAlphaKeyIndex >= strlen(CharSet[inNumberKey]))
					inAlphaKeyIndex = 0;
				break;
			}

			/* 至Array中抓取對應位置的符號再塞到srDispObj->szOutput */
			memcpy(&srDispObj->szOutput[srDispObj->inOutputLen - 1], &CharSet[inNumberKey][inAlphaKeyIndex], 1);

			break;
		case _MENUKEYIN_EVENT_:
			/* 將inMenuKeyIn初始化 */
			srDispObj->inMenuKeyIn = -1;
			break;
		default:
			continue;
		}

		/* 如果長度大於16，將8X16轉8X23顯示 */
		if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) <= 19 && srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 16)
		{
			/* 將8行模式顯示轉換成10行模式 */
			inY_12X19 = (srDispObj->inY * 3) / 2;
			/* 設定螢幕字型大小 */
			srDispObj->inFoneSize = _ENGLISH_FONT_8X22_;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcat(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_12X19_, inY_12X19, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_12X19_, inY_12X19, inColor, _DISP_RIGHT_);
			}
		}
		else if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) <= 22 && srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 19)
		{
			/* 將8行模式顯示轉換成16行模式 */
			inY_16X22 = srDispObj->inY * 2 - 1;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_RIGHT_);
			}
		}
		else if (srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) > 22)
		{
			/* 將8行模式顯示轉換成16行模式 */
			inY_16X22 = srDispObj->inY * 2 - 1;
			inY_16X22_2 = srDispObj->inY * 2;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			inPart1Len = 22 - strlen(srDispObj->szPromptMsg);
			inPart2Len = srDispObj->inOutputLen + strlen(srDispObj->szPromptMsg) - 22;

			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{

					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', inPart1Len);

					memset(szMaskOutput2, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput2, '*', inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
				}
				else
				{
					memset(szTemp, 0x00, sizeof(szTemp));
					memcpy(szTemp, srDispObj->szOutput, inPart1Len);

					memset(szTemp2, 0x00, sizeof(szTemp2));
					memcpy(szTemp2, &srDispObj->szOutput[inPart1Len], inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
					else
					{
						strcpy(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_LEFT_);
				inDISP_EnglishFont_Color(szTemplate2, _FONTSIZE_16X22_, inY_16X22_2, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', inPart1Len);

					memset(szMaskOutput2, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput2, '*', inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
						strcpy(szTemplate2, szMaskOutput2);
					}
				}
				else
				{
					memset(szTemp, 0x00, sizeof(szTemp));
					memcpy(szTemp, srDispObj->szOutput, inPart1Len);

					memset(szTemp2, 0x00, sizeof(szTemp2));
					memcpy(szTemp2, &srDispObj->szOutput[inPart1Len], inPart2Len);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
					else
					{
						strcpy(szTemplate, szTemp);
						strcpy(szTemplate2, szTemp2);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_16X22_, inY_16X22, inColor, _DISP_RIGHT_);
				inDISP_EnglishFont_Color(szTemplate2, _FONTSIZE_16X22_, inY_16X22_2, inColor, _DISP_RIGHT_);
			}
		}
		else
		{
			/* 設定螢幕字型大小 */
			srDispObj->inFoneSize = _ENGLISH_FONT_8X16_;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (srDispObj->inR_L == _DISP_LEFT_)
			{

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcat(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{

				if (srDispObj->inMask == VS_TRUE)
				{
					/* 將要Mask字數存起來，就memset幾個* */
					memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
					memset(szMaskOutput, '*', srDispObj->inOutputLen);

					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, szMaskOutput);
					}
					else
					{
						strcpy(szTemplate, szMaskOutput);
					}
				}
				else
				{
					if (strlen(srDispObj->szPromptMsg) > 0)
					{
						/* 如果有提示字串將顯示 */
						strcpy(szTemplate, srDispObj->szPromptMsg);
						strcat(szTemplate, srDispObj->szOutput);
					}
					else
					{
						strcpy(szTemplate, srDispObj->szOutput);
					}
				}

				inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_RIGHT_);
			}
		}
	}
}

/*
Function	:inDISP_Enter8x16_GetAmount
Date&Time	:2016/9/29 上午 9:53
Describe	:輸入金額
*/
int inDISP_Enter8x16_GetAmount(DISPLAY_OBJECT *srDispObj)
{
	int inChoice = 0;
	int inColor;
	int inFinalTimeOut;
	char szTemplate[32 + 1];
	unsigned char uszkey;

	if (srDispObj->inMenuKeyIn > 0)
	{
		/* 將輸入第一碼先存起來 */
		sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", srDispObj->inMenuKeyIn);
		srDispObj->inOutputLen++;
	}

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	if (srDispObj->inTimeout > 0)
	{
		inFinalTimeOut = srDispObj->inTimeout;
	}
	else
	{
		inFinalTimeOut = _EDC_TIMEOUT_;
	}

	inColor = srDispObj->inColor;

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, inFinalTimeOut);
	while (1)
	{
		uszkey = -1;

		if (srDispObj->inMenuKeyIn > 0)
		{
			/* 如果為MenuKeyIn，第一個數字要顯示 */
			uszkey = _MENUKEYIN_EVENT_;
		}
		else
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(srDispObj->inTouchSensorFunc);
			uszkey = uszKBD_Key();
		}

		if (inChoice == _Touch_OX_LINE8_8_ENTER_BUTTON_)
		{
			uszkey = _KEY_ENTER_;
		}
		else if (inChoice == _Touch_OX_LINE8_8_CANCEL_BUTTON_)
		{
			uszkey = _KEY_CANCEL_;
		}

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszkey = _KEY_TIMEOUT_;
		}

		switch (uszkey)
		{
		case _KEY_CANCEL_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_USER_CANCEL);
		case _KEY_TIMEOUT_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_TIMEOUT);
		case _KEY_ENTER_:
			/* 先確認該部份是否可輸入0 和 ByPass */
			if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				return (srDispObj->inOutputLen);
			}
			/* 不能ByPass但可以輸入0 */
			else if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass == VS_TRUE)
			{
				if (srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0但可以ByPass */
			else if (srDispObj->inCanNotZero == VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0也不能ByPass */
			else
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L || srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
		case _KEY_CLEAR_:
			if (srDispObj->inOutputLen > 0)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;
				break;
			}
			else
				continue;
		case _KEY_0_:
		case _KEY_1_:
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			/* 金額第一位數不能為0 */
			if (srDispObj->inOutputLen == 0 && uszkey - 48 == 0)
			{
				continue;
			}
			/* 若超過最大長度時長嗶一聲 */
			else if (srDispObj->inOutputLen >= srDispObj->inMaxLen)
			{
				inDISP_BEEP(1, 1);
				continue;
			}

			sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", uszkey);
			srDispObj->inOutputLen++;
			break;
		case _MENUKEYIN_EVENT_:
			/* 將inMenuKeyIn初始化 */
			srDispObj->inMenuKeyIn = -1;
			break;
		default:
			continue;
		}

		/* 設定螢幕字型大小 */
		srDispObj->inFoneSize = _ENGLISH_FONT_8X16_;

		/* 一律先把畫面清掉後再顯示輸入訊息 */
		if (srDispObj->inX != 0)
			inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
		else
			inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, srDispObj->szOutput);
		/* 如果砍到沒金額，顯示0元而不是空白 */
		if (strlen(szTemplate) == 0)
		{
			strcat(szTemplate, "0");
		}

		if (srDispObj->inR_L == _DISP_LEFT_)
		{
			inFunc_Amount_Comma(szTemplate, srDispObj->szPromptMsg, 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_);
			inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_LEFT_);
		}
		else if (srDispObj->inR_L == _DISP_RIGHT_)
		{
			inFunc_Amount_Comma(szTemplate, srDispObj->szPromptMsg, 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_);
			inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_RIGHT_);
		}
	}
}

/*
Function	:inDISP_Enter8x16_GetDCCAmount
Date&Time	:2016/9/29 上午 9:26
Describe	:輸入金額 DCC小費用
*/
int inDISP_Enter8x16_GetDCCAmount(DISPLAY_OBJECT *srDispObj)
{
	int inColor;
	int inFinalTimeOut;
	char szTemplate[32 + 1];
	unsigned char uszkey;

	if (srDispObj->inMenuKeyIn > 0)
	{
		/* 將輸入第一碼先存起來 */
		sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", srDispObj->inMenuKeyIn);
		srDispObj->inOutputLen++;
	}

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	if (srDispObj->inTimeout > 0)
	{
		inFinalTimeOut = srDispObj->inTimeout;
	}
	else
	{
		inFinalTimeOut = _EDC_TIMEOUT_;
	}

	inColor = srDispObj->inColor;

	while (1)
	{
		uszkey = -1;

		if (srDispObj->inMenuKeyIn > 0)
		{
			/* 如果為MenuKeyIn，第一個數字要顯示 */
			uszkey = _MENUKEYIN_EVENT_;
		}
		else
		{
			uszkey = uszKBD_GetKey(inFinalTimeOut);
		}

		switch (uszkey)
		{
		case _KEY_CANCEL_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_USER_CANCEL);
		case _KEY_TIMEOUT_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_TIMEOUT);
		case _KEY_ENTER_:
			/* 先確認該部份是否可輸入0 和 ByPass */
			if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				return (srDispObj->inOutputLen);
			}
			/* 不能ByPass但可以輸入0 */
			else if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass == VS_TRUE)
			{
				if (srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0但可以ByPass */
			else if (srDispObj->inCanNotZero == VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0也不能ByPass */
			else
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L || srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
		case _KEY_CLEAR_:
			if (srDispObj->inOutputLen > 0)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;
				break;
			}
			else
				continue;
		case _KEY_0_:
		case _KEY_1_:
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			/* 金額第一位數不能為0 */
			if (srDispObj->inOutputLen == 0 && uszkey - 48 == 0)
			{
				continue;
			}
			/* 若超過最大長度時長嗶一聲 */
			else if (srDispObj->inOutputLen >= srDispObj->inMaxLen)
			{
				inDISP_BEEP(1, 1);
				continue;
			}

			sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", uszkey);
			srDispObj->inOutputLen++;
			break;
		case _MENUKEYIN_EVENT_:
			/* 將inMenuKeyIn初始化 */
			srDispObj->inMenuKeyIn = -1;
			break;
		default:
			continue;
		}

		/* 設定螢幕字型大小 */
		srDispObj->inFoneSize = _ENGLISH_FONT_8X16_;

		/* 一律先把畫面清掉後再顯示輸入訊息 */
		if (srDispObj->inX != 0)
			inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
		else
			inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, srDispObj->szOutput);
		/* 如果砍到沒金額，顯示0元而不是空白 */
		if (strlen(szTemplate) == 0)
		{
			strcat(szTemplate, "0");
		}

		if (srDispObj->inR_L == _DISP_LEFT_)
		{
			inFunc_Amount_Comma_DCC(szTemplate, srDispObj->szPromptMsg, 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_, srDispObj->szMinorUnit, srDispObj->szCurrencyCode, szTemplate);
			inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_LEFT_);
		}
		else if (srDispObj->inR_L == _DISP_RIGHT_)
		{
			/* 將輸入的字右靠左補空白 */
			inFunc_Amount_Comma_DCC(szTemplate, srDispObj->szPromptMsg, 0x00, _SIGNED_NONE_, 16, _PAD_LEFT_FILL_RIGHT_, srDispObj->szMinorUnit, srDispObj->szCurrencyCode, szTemplate);
			inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj->inY, inColor, _DISP_RIGHT_);
		}
	}
}

/*
Function	:inDISP_Enter8x23
Date&Time	:2017/4/26 下午 3:32
Describe	:輸入字元
*/
int inDISP_Enter8x23(DISPLAY_OBJECT *srDispObj)
{
	int inColor;
	int inFinalTimeOut;
	int inNumberKey = -1, inAlphaKeyIndex = 1;
	char szTemplate[_DISP_MSG_SIZE_ + 1], szMaskOutput[_DISP_MSG_SIZE_ + 1];
	unsigned char uszkey;

	memset(szMaskOutput, 0x00, sizeof(szMaskOutput));

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	if (srDispObj->inTimeout > 0)
	{
		inFinalTimeOut = srDispObj->inTimeout;
	}
	else
	{
		inFinalTimeOut = _EDC_TIMEOUT_;
	}

	inColor = srDispObj->inColor;

	while (1)
	{
		uszkey = -1;

		if (srDispObj->inMenuKeyIn > 0)
		{
			/* 如果為MenuKeyIn，第一個數字要顯示 */
			uszkey = _MENUKEYIN_EVENT_;
		}
		else
		{
			uszkey = uszKBD_GetKey(inFinalTimeOut);
		}

		switch (uszkey)
		{
		case _KEY_CANCEL_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_USER_CANCEL);
		case _KEY_TIMEOUT_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_TIMEOUT);
		case _KEY_ENTER_:
			/* 先確認該部份是否可輸入0 和 ByPass */
			if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				return (srDispObj->inOutputLen);
			}
			/* 不能ByPass但可以輸入0 */
			else if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass == VS_TRUE)
			{
				if (srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0但可以ByPass */
			else if (srDispObj->inCanNotZero == VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0也不能ByPass */
			else
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L || srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
		case _KEY_CLEAR_:
			inAlphaKeyIndex = 1;
			inNumberKey = -1;

			if (srDispObj->inOutputLen > 0)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				szMaskOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;
				break;
			}
			else
				continue;
		case _KEY_0_:
		case _KEY_1_:
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			/* 若超過最大長度時長嗶一聲 */
			if (srDispObj->inOutputLen >= srDispObj->inMaxLen)
			{
				inDISP_BEEP(1, 1);
				continue;
			}

			sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", uszkey);
			srDispObj->inOutputLen++;

			/* 將當下的Array存起來，以便切換Alpha鍵 */
			inNumberKey = uszkey - 48;
			/* 回到當初的index */
			inAlphaKeyIndex = 1;
			break;
		case _MENUKEYIN_EVENT_:
			/* 將inMenuKeyIn初始化 */
			srDispObj->inMenuKeyIn = -1;
			break;
		default:
			continue;
		}

		/* 設定螢幕字型大小 */
		srDispObj->inFoneSize = _ENGLISH_FONT_8X22_;
		/* 一律先把畫面清掉後再顯示輸入訊息 */
		inDISP_Clear_Area(1, srDispObj->inY, 23, srDispObj->inY, srDispObj->inFoneSize);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (srDispObj->inR_L == _DISP_LEFT_)
		{
			if (srDispObj->inMask == VS_TRUE)
			{
				/* 將要Mask字數存起來，就memset幾個* */
				memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
				memset(szMaskOutput, '*', srDispObj->inOutputLen);

				if (strlen(srDispObj->szPromptMsg) > 0)
				{
					/* 如果有提示字串將顯示 */
					strcpy(szTemplate, srDispObj->szPromptMsg);
					strcat(szTemplate, szMaskOutput);
				}
				else
				{
					strcpy(szTemplate, szMaskOutput);
				}
			}
			else
			{
				if (strlen(srDispObj->szPromptMsg) > 0)
				{
					/* 如果有提示字串將顯示 */
					strcpy(szTemplate, srDispObj->szPromptMsg);
					strcat(szTemplate, srDispObj->szOutput);
				}
				else
				{
					strcpy(szTemplate, srDispObj->szOutput);
				}
			}

			inDISP_EnglishFont_Color(srDispObj->szOutput, _FONTSIZE_8X22_, srDispObj->inY, inColor, _DISP_LEFT_);
		}
		else if (srDispObj->inR_L == _DISP_RIGHT_)
		{
			if (srDispObj->inMask == VS_TRUE)
			{
				/* 將要Mask字數存起來，就memset幾個* */
				memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
				memset(szMaskOutput, '*', srDispObj->inOutputLen);

				if (strlen(srDispObj->szPromptMsg) > 0)
				{
					/* 如果有提示字串將顯示 */
					strcpy(szTemplate, srDispObj->szPromptMsg);
					strcat(szTemplate, szMaskOutput);
				}
				else
				{
					strcpy(szTemplate, szMaskOutput);
				}
			}
			else
			{
				if (strlen(srDispObj->szPromptMsg) > 0)
				{
					/* 如果有提示字串將顯示 */
					strcpy(szTemplate, srDispObj->szPromptMsg);
					strcat(szTemplate, srDispObj->szOutput);
				}
				else
				{
					strcpy(szTemplate, srDispObj->szOutput);
				}
			}

			inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X22_, srDispObj->inY, inColor, _DISP_RIGHT_);
		}
	}
}

/*
Function	:inDISP_Enter8x23_Character_Mask
Date&Time	:2015/8/12 上午 10:03
Describe	:輸入字元包含Mask
*/
int inDISP_Enter8x23_Character_Mask(DISPLAY_OBJECT *srDispObj)
{
	int inColor;
	int inFinalTimeOut;
	int inNumberKey = -1, inAlphaKeyIndex = 0;
	char szTemplate[_DISP_MSG_SIZE_ + 1], szMaskOutput[_DISP_MSG_SIZE_ + 1];
	char CharSet[10][15] = {{"0.,@* -"}, {"1QZqz"}, {"2ABCabc"}, {"3DEFdef"}, {"4GHIghi"}, {"5JKLjkl"}, {"6MNOmno"}, {"7PRSprs"}, {"8TUVtuv"}, {"9WXYwxy"}};
	unsigned char uszkey = 0x00;
	unsigned char uszFindAlpha = VS_FALSE;

	memset(szMaskOutput, 0x00, sizeof(szMaskOutput));

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	if (srDispObj->inTimeout > 0)
	{
		inFinalTimeOut = srDispObj->inTimeout;
	}
	else
	{
		inFinalTimeOut = _EDC_TIMEOUT_;
	}

	inColor = srDispObj->inColor;

	while (1)
	{
		uszkey = -1;

		if (srDispObj->inMenuKeyIn > 0)
		{
			/* 如果為MenuKeyIn，第一個數字要顯示 */
			uszkey = _MENUKEYIN_EVENT_;
		}
		else
		{
			uszkey = uszKBD_GetKey(inFinalTimeOut);
		}

		switch (uszkey)
		{
		case _KEY_CANCEL_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_USER_CANCEL);
		case _KEY_TIMEOUT_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_TIMEOUT);
		case _KEY_ENTER_:
			/* 先確認該部份是否可輸入0 和 ByPass */
			if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				return (srDispObj->inOutputLen);
			}
			/* 不能ByPass但可以輸入0 */
			else if (srDispObj->inCanNotZero != VS_TRUE && srDispObj->inCanNotBypass == VS_TRUE)
			{
				if (srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0但可以ByPass */
			else if (srDispObj->inCanNotZero == VS_TRUE && srDispObj->inCanNotBypass != VS_TRUE)
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
			/* 不能輸入0也不能ByPass */
			else
			{
				/* 判斷輸入為零 提示聲音+重新輸入 */
				if (atol(srDispObj->szOutput) == 0L || srDispObj->inOutputLen == 0)
				{
					inDISP_BEEP(1, 0);
					continue;
				}
				else
				{
					return (srDispObj->inOutputLen);
				}
			}
		case _KEY_CLEAR_:
			if (srDispObj->inOutputLen > 1)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				szMaskOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;

				uszFindAlpha = VS_FALSE;
				for (inNumberKey = 0; inNumberKey < 10; inNumberKey++)
				{
					for (inAlphaKeyIndex = 0; inAlphaKeyIndex < 15; inAlphaKeyIndex++)
					{
						if (CharSet[inNumberKey][inAlphaKeyIndex] == srDispObj->szOutput[srDispObj->inOutputLen - 1])
						{
							uszFindAlpha = VS_TRUE;
						}

						/* 找到就一路break出去 */
						if (uszFindAlpha == VS_TRUE)
						{
							break;
						}
					}

					/* 找到就一路break出去 */
					if (uszFindAlpha == VS_TRUE)
					{
						break;
					}
				}

				/* 找到就一路break出去 */
				if (uszFindAlpha == VS_TRUE)
				{
				}
				else
				{
					inAlphaKeyIndex = 0;
					inNumberKey = -1;
				}
				break;
			}
			else if (srDispObj->inOutputLen == 1)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				szMaskOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;

				inAlphaKeyIndex = 0;
				inNumberKey = -1;
				break;
			}
			else
			{
				inAlphaKeyIndex = 0;
				inNumberKey = -1;
				continue;
			}
		case _KEY_0_:
		case _KEY_1_:
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			/* 若超過最大長度時長嗶一聲 */
			if (srDispObj->inOutputLen >= srDispObj->inMaxLen)
			{
				inDISP_BEEP(1, 1);
				continue;
			}

			sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", uszkey);
			srDispObj->inOutputLen++;

			/* 將當下的Array存起來，以便切換Alpha鍵 */
			inNumberKey = uszkey - 48;
			/* 回到當初的index */
			inAlphaKeyIndex = 0;
			break;
		case _KEY_ALPHA_:
			if (inNumberKey == -1)
				continue;

			inAlphaKeyIndex++;

			switch (inNumberKey)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				if (inAlphaKeyIndex >= strlen(CharSet[inNumberKey]))
					inAlphaKeyIndex = 0;
				break;
			}

			/* 至Array中抓取對應位置的符號再塞到srDispObj->szOutput */
			memcpy(&srDispObj->szOutput[srDispObj->inOutputLen - 1], &CharSet[inNumberKey][inAlphaKeyIndex], 1);

			break;
		case _MENUKEYIN_EVENT_:
			/* 將inMenuKeyIn初始化 */
			srDispObj->inMenuKeyIn = -1;
			break;
		default:
			continue;
		}

		/* 設定螢幕字型大小 */
		srDispObj->inFoneSize = _ENGLISH_FONT_8X22_;
		/* 一律先把畫面清掉後再顯示輸入訊息 */
		inDISP_Clear_Area(1, srDispObj->inY, 23, srDispObj->inY, srDispObj->inFoneSize);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (srDispObj->inR_L == _DISP_LEFT_)
		{
			if (srDispObj->inMask == VS_TRUE)
			{
				/* 將要Mask字數存起來，就memset幾個* */
				memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
				memset(szMaskOutput, '*', srDispObj->inOutputLen);

				if (strlen(srDispObj->szPromptMsg) > 0)
				{
					/* 如果有提示字串將顯示 */
					strcpy(szTemplate, srDispObj->szPromptMsg);
					strcat(szTemplate, szMaskOutput);
				}
				else
				{
					strcpy(szTemplate, szMaskOutput);
				}
			}
			else
			{
				if (strlen(srDispObj->szPromptMsg) > 0)
				{
					/* 如果有提示字串將顯示 */
					strcpy(szTemplate, srDispObj->szPromptMsg);
					strcat(szTemplate, srDispObj->szOutput);
				}
				else
				{
					strcpy(szTemplate, srDispObj->szOutput);
				}
			}

			inDISP_EnglishFont_Color(srDispObj->szOutput, _FONTSIZE_8X22_, srDispObj->inY, inColor, _DISP_LEFT_);
		}
		else if (srDispObj->inR_L == _DISP_RIGHT_)
		{
			if (srDispObj->inMask == VS_TRUE)
			{
				/* 將要Mask字數存起來，就memset幾個* */
				memset(szMaskOutput, 0x00, sizeof(szMaskOutput));
				memset(szMaskOutput, '*', srDispObj->inOutputLen);

				if (strlen(srDispObj->szPromptMsg) > 0)
				{
					/* 如果有提示字串將顯示 */
					strcpy(szTemplate, srDispObj->szPromptMsg);
					strcat(szTemplate, szMaskOutput);
				}
				else
				{
					strcpy(szTemplate, szMaskOutput);
				}
			}
			else
			{
				if (strlen(srDispObj->szPromptMsg) > 0)
				{
					/* 如果有提示字串將顯示 */
					strcpy(szTemplate, srDispObj->szPromptMsg);
					strcat(szTemplate, srDispObj->szOutput);
				}
				else
				{
					strcpy(szTemplate, srDispObj->szOutput);
				}
			}

			inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X22_, srDispObj->inY, inColor, _DISP_RIGHT_);
		}
	}
}

/*
Function	:inDISP_DisplayIdleMessage
Date&Time	:2015/8/2 下午 4:28
Describe	:IDLE畫面顯示
*/
int inDISP_DisplayIdleMessage(void)
{
	int inFunc = 0;
	int inTouchSensorFunc;

	/* 開啟IDLE畫面的偵測 */
	inTouchSensorFunc = _Touch_IDLE_;
	inFunc = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);

	/*
	 * _IdleTouch_KEY_1_: 開啟MENU畫面觸控
	 * _IdleTouch_KEY_2_: 開啟銀聯交易畫面觸控
	 * _IdleTouch_KEY_3_: 開啟其他交易畫面觸控
	 */

	return (inFunc);
}

/*
Function	: inDISP_DisplayIdleMessage_NewUI
Date&Time	:2017/10/16 上午 11:43
Describe	: IDLE畫面顯示
*/
int inDISP_DisplayIdleMessage_NewUI(void)
{
	int inFunc = 0;
	int inTouchSensorFunc = _Touch_NONE_;

	/* 開啟IDLE畫面的偵測 */
	inTouchSensorFunc = _Touch_NEWUI_IDLE_;
	inFunc = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);

	/*
	 * _IdleTouch_KEY_1_: 開啟MENU畫面觸控
	 * _IdleTouch_KEY_2_: 開啟銀聯交易畫面觸控
	 * _IdleTouch_KEY_3_: 開啟其他交易畫面觸控
	 */

	return (inFunc);
}

/*
Function	:inDISP_ErrorMsg
Date&Time	:2015/8/21 下午 1:29
Describe	:顯示ERROR MSG
*/
int inDISP_ErrorMsg(DISPLAY_OBJECT *srDispObj)
{
	int inFinalTimeOut;
	unsigned char uszKey;

	/* 清下排 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	inDISP_ChineseFont(srDispObj->szErrMsg1, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
	inDISP_ChineseFont(srDispObj->szErrMsg2, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	if (srDispObj->inTimeout > 0)
	{
		inFinalTimeOut = srDispObj->inTimeout;
	}
	else
	{
		inFinalTimeOut = _EDC_TIMEOUT_;
	}

	if (srDispObj->inMsgType == _CLEAR_KEY_MSG_)
	{
		inDISP_ChineseFont_Color("請按清除鍵", _FONTSIZE_8X16_, _LINE_8_7_, _COLOR_RED_, _DISP_LEFT_);
		inDISP_BEEP(1, 0);

		while (1)
		{
			uszKey = uszKBD_GetKey(inFinalTimeOut);

			if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				break;
			}
			else
			{
				continue;
			}
		}
	}
	else if (srDispObj->inMsgType == _BEEP_3TIMES_MSG_)
		inDISP_BEEP(3, 500);
	else
		inDISP_Wait(3000);

	return (VS_SUCCESS);
}

/*
Function	:inDISP_Msg_BMP
Date&Time	:2016/2/23 下午 4:16
Describe	:圖片提示並指定清除鍵確認鍵或聲音提示
 *szAdditonalMsg:當有要顯示額外資訊的時候,不顯示時，填入""，inAdditonalMsgLine，填入0
*/
int inDISP_Msg_BMP(char *szFileName, int inYPosition, int inMsgType, int inTimeOut, char *szAdditonalMsg, int inAdditonalMsgLine)
{

	int inRetVal = VS_SUCCESS;
	int inFinalTimeOut = 0;
	char szCustomerIndicator[3 + 1] = {0};
	unsigned char uszKey;

	/* 清下排 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	if (strlen(szFileName) > 0)
	{
		inDISP_PutGraphic(szFileName, 0, inYPosition);
	}

	if (inAdditonalMsgLine != 0)
	{
		inDISP_ChineseFont(szAdditonalMsg, _FONTSIZE_8X16_, inAdditonalMsgLine, _DISP_CENTER_);
	}

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	/* 客製化107需求，顯示錯誤訊息Timeout2秒 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, strlen(_CUSTOMER_INDICATOR_107_BUMPER_)))
	{
		if (inMsgType == _CLEAR_KEY_MSG_ ||
			inMsgType == _ENTER_KEY_MSG_ ||
			inMsgType == _0_KEY_MSG_)
		{
			inFinalTimeOut = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
		}
		else
		{
			if (inTimeOut > 0)
			{
				inFinalTimeOut = inTimeOut;
			}
			else
			{
				inFinalTimeOut = _EDC_TIMEOUT_;
			}
		}
	}
	else
	{
		if (inTimeOut > 0)
		{
			inFinalTimeOut = inTimeOut;
		}
		else
		{
			inFinalTimeOut = _EDC_TIMEOUT_;
		}
	}

	/* 抓取 KioskFlag的值，如為 TRUE 就不用確認畫面 2020/3/6 下午 4:28 [SAM] */
	if (inFunc_GetKisokFlag() == VS_TRUE)
	{
		if (inMsgType == _CLEAR_KEY_MSG_)
		{
			inDISP_BEEP(1, 0);
			/* 因不需要等待，所以都停一秒 */
			inDISP_Wait(1000);
			inRetVal = VS_USER_CANCEL;
		}
		else if (inMsgType == _ENTER_KEY_MSG_)
		{
			inDISP_BEEP(1, 0);
			/* 因不需要等待，所以都停一秒 */
			inDISP_Wait(1000);
		}
		else if (inMsgType == _0_KEY_MSG_)
		{
			inDISP_BEEP(1, 0);
			/* 因不需要等待，所以都停一秒 */
			inDISP_Wait(1000);
		}
		else if (inMsgType == _BEEP_1TIMES_MSG_)
		{
			inDISP_BEEP(1, inFinalTimeOut * 1000);
		}
		else if (inMsgType == _BEEP_3TIMES_MSG_)
		{
			inDISP_BEEP(3, inFinalTimeOut * 1000);
		}
		else if (inMsgType == _NO_KEY_MSG_)
		{
			inDISP_Wait(inFinalTimeOut * 1000);
		}

		inDISP_LogPrintfWithFlag("  Kiosk DispMsgBmp Line[%d]", __LINE__);
	}
	else
	{

		if (inMsgType == _CLEAR_KEY_MSG_)
		{
			inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);
			inDISP_BEEP(1, 0);

			while (1)
			{
				uszKey = uszKBD_GetKey(inFinalTimeOut);

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
				else
				{
					continue;
				}
			}
		}
		else if (inMsgType == _ENTER_KEY_MSG_)
		{
			inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
			inDISP_BEEP(1, 0);

			while (1)
			{
				uszKey = uszKBD_GetKey(inFinalTimeOut);

				if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else if (uszKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					break;
				}
				else
				{
					continue;
				}
			}
		}
		else if (inMsgType == _0_KEY_MSG_)
		{
			inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
			inDISP_BEEP(1, 0);

			while (1)
			{
				uszKey = uszKBD_GetKey(inFinalTimeOut);

				if (uszKey == _KEY_0_)
				{
					break;
				}
				else if (uszKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					break;
				}
				else
				{
					continue;
				}
			}
		}
		else if (inMsgType == _BEEP_1TIMES_MSG_)
		{
			inDISP_BEEP(1, inFinalTimeOut * 1000);
		}
		else if (inMsgType == _BEEP_3TIMES_MSG_)
		{
			inDISP_BEEP(3, inFinalTimeOut * 1000);
		}
		else if (inMsgType == _NO_KEY_MSG_)
		{
			inDISP_Wait(inFinalTimeOut * 1000);
		}
	}
	/* 離開時再清一次下排 把錯誤訊息洗掉*/
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	return (inRetVal);
}

/*
Function	: inDISP_Msg_NewBMP
Date&Time : 2022/6/7 下午 5:20
Describe	:圖片提示並指定清除鍵確認鍵或聲音提示
 *szAdditonalMsg:當有要顯示額外資訊的時候,不顯示時，填入""，inAdditonalMsgLine，填入0
 * [新增電票悠遊卡功能]  [SAM] 2022/6/8 下午 5:56
*/
int inDISP_Msg_NewBMP(DISPLAY_OBJECT *srDispMsgObj)
{
	int inOrgMsgType = _NO_KEY_MSG_; /* For 107 or 111這種會中途改變MsgType使用 */
	int inRetVal = VS_SUCCESS;
	int inFinalTimeOut = 0;
	int inFontSize1 = 0;
	int inFontSize2 = 0;
	int inFontSize3 = 0;
	char szCustomerIndicator[3 + 1] = {0};
	char szTimeOut[3 + 1] = {0};
	unsigned char uszKey;

	/* 清下排 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	if (strlen(srDispMsgObj->szDispPic1Name) > 0)
	{
		inDISP_PutGraphic(srDispMsgObj->szDispPic1Name, srDispMsgObj->inDispPic1XPosition, srDispMsgObj->inDispPic1YPosition);
	}

	if (strlen(srDispMsgObj->szErrMsg1) > 0)
	{
		if (srDispMsgObj->inErrMsg1FontSize > 0)
		{
			inFontSize1 = srDispMsgObj->inErrMsg1FontSize;
		}
		else
		{
			inFontSize1 = _FONTSIZE_8X16_;
		}

		inDISP_ChineseFont(srDispMsgObj->szErrMsg1, inFontSize1, srDispMsgObj->inErrMsg1Line, _DISP_CENTER_);
	}

	if (strlen(srDispMsgObj->szErrMsg2) > 0)
	{
		if (srDispMsgObj->inErrMsg2FontSize > 0)
		{
			inFontSize2 = srDispMsgObj->inErrMsg2FontSize;
		}
		else
		{
			inFontSize2 = _FONTSIZE_8X16_;
		}

		inDISP_ChineseFont(srDispMsgObj->szErrMsg2, inFontSize2, srDispMsgObj->inErrMsg2Line, _DISP_CENTER_);
	}

	if (strlen(srDispMsgObj->szErrMsg3) > 0)
	{
		if (srDispMsgObj->inErrMsg3FontSize > 0)
		{
			inFontSize3 = srDispMsgObj->inErrMsg3FontSize;
		}
		else
		{
			inFontSize3 = _FONTSIZE_8X16_;
		}
		inDISP_ChineseFont(srDispMsgObj->szErrMsg3, inFontSize3, srDispMsgObj->inErrMsg3Line, _DISP_CENTER_);
	}

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	{
		if (srDispMsgObj->inTimeout >= 0)
		{
			inFinalTimeOut = srDispMsgObj->inTimeout;
		}
		else
		{
			memset(szTimeOut, 0x00, sizeof(szTimeOut));
			inGetEnterTimeout(szTimeOut);
			inFinalTimeOut = atoi(szTimeOut);
		}
	}

	if (srDispMsgObj->inMsgType == _CLEAR_KEY_MSG_)
	{
		inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);
		if (srDispMsgObj->inBeepTimes > 0)
		{
			inDISP_BEEP(srDispMsgObj->inBeepTimes, srDispMsgObj->inBeepInterval);
		}

		while (1)
		{
			uszKey = uszKBD_GetKey(inFinalTimeOut);

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
			else
			{
				continue;
			}
		}
	}
	else if (srDispMsgObj->inMsgType == _ENTER_KEY_MSG_)
	{
		inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
		if (srDispMsgObj->inBeepTimes > 0)
		{
			inDISP_BEEP(srDispMsgObj->inBeepTimes, srDispMsgObj->inBeepInterval);
		}

		while (1)
		{
			uszKey = uszKBD_GetKey(inFinalTimeOut);

			if (uszKey == _KEY_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (uszKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
			else
			{
				continue;
			}
		}
	}
	else if (srDispMsgObj->inMsgType == _0_KEY_MSG_)
	{
		inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
		if (srDispMsgObj->inBeepTimes > 0)
		{
			inDISP_BEEP(srDispMsgObj->inBeepTimes, srDispMsgObj->inBeepInterval);
		}

		while (1)
		{
			uszKey = uszKBD_GetKey(inFinalTimeOut);

			if (uszKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (uszKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
			else
			{
				continue;
			}
		}
	}
	else if (srDispMsgObj->inMsgType == _ANY_KEY_MSG_)
	{
		inDISP_ChineseFont("請按任意鍵", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);
		if (srDispMsgObj->inBeepTimes > 0)
		{
			inDISP_BEEP(srDispMsgObj->inBeepTimes, srDispMsgObj->inBeepInterval);
		}

		while (1)
		{
			uszKey = uszKBD_GetKey(inFinalTimeOut);

			if (uszKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
			else
			{
				inRetVal = VS_SUCCESS;
				break;
			}
		}
	}
	else if (srDispMsgObj->inMsgType == _NO_KEY_MSG_)
	{
		if (srDispMsgObj->inBeepTimes > 0)
		{
			inDISP_BEEP(srDispMsgObj->inBeepTimes, srDispMsgObj->inBeepInterval);
			inDISP_Wait(inFinalTimeOut * 1000);
		}
		else
		{
			inDISP_Wait(inFinalTimeOut * 1000);
		}

		if (inOrgMsgType == _CLEAR_KEY_MSG_)
		{
			inRetVal = VS_USER_CANCEL;
		}
		else if (inOrgMsgType == _ENTER_KEY_MSG_)
		{
			inRetVal = VS_SUCCESS;
		}
		else if (inOrgMsgType == _ANY_KEY_MSG_)
		{
			inRetVal = VS_SUCCESS;
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}

	/* 離開時再清一次下排 把錯誤訊息洗掉*/
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	return (inRetVal);
}

/*
Function	:inDISP_Ente8x14_MenuKeyIn
Date&Time	:2015/9/11 下午 8:13
Describe	:輸入數字
Note            :輸入超過長度大於16時，顯示會轉換為9X19
*/
int inDISP_Enter8x16_MenuKeyIn(DISPLAY_OBJECT *srDispObj)
{
	int inY_12X19;
	int inFinalTimeOut;
	unsigned char uszkey;

	if (srDispObj->inMenuKeyIn > 0)
	{
		/* 將輸入第一碼先存起來 */
		sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", srDispObj->inMenuKeyIn);
		srDispObj->inOutputLen++;
	}

	/* 若TIMEOUT時間大於0時用傳進來的TimeOut，否則用EDC.dat的 */
	if (srDispObj->inTimeout > 0)
	{
		inFinalTimeOut = srDispObj->inTimeout;
	}
	else
	{
		inFinalTimeOut = _EDC_TIMEOUT_;
	}

	while (1)
	{
		uszkey = -1;

		if (srDispObj->inMenuKeyIn > 0)
		{
			/* 如果為MenuKeyIn，第一個數字要顯示 */
			uszkey = _MENUKEYIN_EVENT_;
		}
		else
		{
			uszkey = uszKBD_GetKey(inFinalTimeOut);
		}

		switch (uszkey)
		{
		case _KEY_CANCEL_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			return (VS_USER_CANCEL);
		case _KEY_TIMEOUT_:
			srDispObj->inOutputLen = 0;
			memset(srDispObj->szOutput, 0x00, sizeof(srDispObj->szOutput));
			// inDISP_Msg_BMP(_ERR_PLEASE_FOLLOW_RIGHT_OPT_, _COORDINATE_Y_LINE_8_5_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
			return (VS_TIMEOUT);
		case _KEY_ENTER_:
			return (srDispObj->inOutputLen);
		case _KEY_CLEAR_:
			if (srDispObj->inOutputLen > 0)
			{
				srDispObj->szOutput[srDispObj->inOutputLen - 1] = 0x00;
				srDispObj->inOutputLen--;
				break;
			}
			else
				continue;
		case _KEY_0_:
		case _KEY_1_:
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			/* 若超過最大長度時長嗶一聲 */
			if (srDispObj->inOutputLen >= srDispObj->inMaxLen)
			{
				inDISP_BEEP(1, 1);
				continue;
			}

			sprintf(&srDispObj->szOutput[srDispObj->inOutputLen], "%c", uszkey);
			srDispObj->inOutputLen++;
			break;
		case _MENUKEYIN_EVENT_:
			/* 將inMenuKeyIn初始化 */
			srDispObj->inMenuKeyIn = -1;
			break;
		default:
			continue;
		}

		/* 如果長度大於16，將8X16轉12X19顯示 */
		if (srDispObj->inOutputLen > 16)
		{
			/* 將4行模式顯示轉換成10行模式 */
			inY_12X19 = _LINE_12_10_;
			/* 設定螢幕字型大小 */
			srDispObj->inFoneSize = _ENGLISH_FONT_8X16_;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				inDISP_EnglishFont_Color(srDispObj->szOutput, _FONTSIZE_12X19_, inY_12X19, _COLOR_RED_, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				inDISP_EnglishFont_Color(srDispObj->szOutput, _FONTSIZE_12X19_, inY_12X19, _COLOR_RED_, _DISP_RIGHT_);
			}
		}
		else
		{
			/* 設定螢幕字型大小 */
			srDispObj->inFoneSize = _ENGLISH_FONT_8X16_;
			/* 一律先把畫面清掉後再顯示輸入訊息 */
			if (srDispObj->inX != 0)
				inDISP_Clear_Area(srDispObj->inX, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);
			else
				inDISP_Clear_Area(1, srDispObj->inY, 16, srDispObj->inY, srDispObj->inFoneSize);

			if (srDispObj->inR_L == _DISP_LEFT_)
			{
				inDISP_EnglishFont_Color(srDispObj->szOutput, _FONTSIZE_8X16_, srDispObj->inY, _COLOR_RED_, _DISP_LEFT_);
			}
			else if (srDispObj->inR_L == _DISP_RIGHT_)
			{
				inDISP_EnglishFont_Color(srDispObj->szOutput, _FONTSIZE_8X16_, srDispObj->inY, _COLOR_RED_, _DISP_RIGHT_);
			}
		}
	}
}

/*
Function        :inDISP_Timer_Start
Date&Time       :2017/2/2 下午 4:34
Describe        :當lnDelay設為-1，使用EDC.dat內的TimeOut
*/
int inDISP_Timer_Start(int inTimerNumber, long lnDelay)
{
	int inEnterTimeout = 0;
	char szEnterTimeout[3 + 1];

	if (lnDelay == _EDC_TIMEOUT_)
	{
		memset(szEnterTimeout, 0x00, sizeof(szEnterTimeout));
		inGetEnterTimeout(szEnterTimeout);
		inEnterTimeout = atoi(szEnterTimeout);
		if (inEnterTimeout != 0)
		{
			inTimerStart(inTimerNumber, (long)inEnterTimeout);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	else
	{
		inTimerStart(inTimerNumber, lnDelay);
	}

	return (VS_SUCCESS);
}

/*
Function        :inDISP_Timer_Start_MicroSecond
Date&Time       :2018/2/9 下午 1:57
Describe        :當lnDelay設為-1，使用EDC.dat內的TimeOut，時間單位更細的API
*/
int inDISP_Timer_Start_MicroSecond(int inTimerNumber, long lnDelay)
{
	int inEnterTimeout = 0;
	char szEnterTimeout[3 + 1];

	if (lnDelay == _EDC_TIMEOUT_)
	{
		memset(szEnterTimeout, 0x00, sizeof(szEnterTimeout));
		inGetEnterTimeout(szEnterTimeout);
		inEnterTimeout = atoi(szEnterTimeout);
		if (inEnterTimeout != 0)
		{
			inTimerStart_MicroSecond(inTimerNumber, (long)inEnterTimeout);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	else
	{
		inTimerStart_MicroSecond(inTimerNumber, lnDelay);
	}

	return (VS_SUCCESS);
}

/*
Function        :inDISP_LOGDisplay
Date&Time       :2017/7/3 下午 1:40
Describe        :顯示Debug
*/
int inDISP_LOGDisplay(char *szString, int inFontSize, int inLine, unsigned char uszStop)
{
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont(szString, inFontSize, inLine, _DISP_LEFT_);

	if (uszStop == VS_TRUE)
	{
		uszKBD_GetKey(_EDC_TIMEOUT_);
	}

	return (VS_SUCCESS);
}

/*
Function	:inDISP_LogPrintf
Date&Time	:2016/11/30 下午 5:43
Describe	:Debug Printf
*/
int inDISP_LogPrintf(char *szStr, ...)
{
	char szTemplate[20 + 1] = {0};
	char szDebugMsg[15000 + 1] = {0};
	//	char		szDebugMsg[5000 + 1] = {0};
	RTC_NEXSYS srRTC = {0};
	va_list list; /* 加入不定參數作法 2018/5/29 上午 11:06 */

	/* 加入測試訊息時間 2018/3/13 上午 10:19 */
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szTemplate, "%02d-%02d-%02d %02d:%02d:%02d ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	printf(szTemplate);

	va_start(list, szStr);
	vsprintf(szDebugMsg, szStr, list);
	va_end(list);
	printf(szDebugMsg);
	printf("\n");

	return (VS_SUCCESS);
}
void inDISP_LogPrintfArea(char fDebugFlag, char *chMsg, int inMsgLen, BYTE *chData, int inDataLen)
{
	BYTE *bBuf;
	int i;

	if (ginDebug == VS_TRUE)
	{
		bBuf = (BYTE *)calloc(1, inDataLen * 4 + inMsgLen + 10);
		memset(bBuf, 0x00, inDataLen * 3 + inMsgLen + 10);
		memcpy(bBuf, chMsg, inMsgLen);
		sprintf((char *)&bBuf[strlen((char *)bBuf)], " (%d):", inDataLen);
		for (i = 0; i < inDataLen; i++)
		{
			if (fDebugFlag == FALSE)
			{
				if (chData[i] >= 0x20 && chData[i] <= 0x7F)
					bBuf[strlen((char *)bBuf)] = chData[i];
				else
					sprintf((char *)&bBuf[strlen((char *)bBuf)], "[%02X]", chData[i]);
			}
			else
				sprintf((char *)&bBuf[strlen((char *)bBuf)], "%02X", chData[i]);
		}
		inDISP_LogPrintf((char *)bBuf);
		free(bBuf);
	}
}

/*
Function        :inDISP_LogPrintf_Format
Date&Time       :2017/1/10 上午 10:35
Describe        :達到一定長度後自動切斷
*/
int inDISP_LogPrintf_Format(char *szPrintBuffer, char *szPadData, int inOneLineLen)
{
	int inPrintLineCnt = 0;
	char szPrtBuf[50 + 1], szPrintLineData[36 + 1];

	inPrintLineCnt = 0;
	while ((inPrintLineCnt * inOneLineLen) < strlen(szPrintBuffer))
	{
		memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
		memset(szPrtBuf, 0x00, sizeof(szPrtBuf));
		if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szPrintBuffer))
		{
			strcat(szPrintLineData, &szPrintBuffer[inPrintLineCnt * inOneLineLen]);
		}
		else
		{
			memcpy(szPrintLineData, &szPrintBuffer[inPrintLineCnt * inOneLineLen], inOneLineLen);
		}

		sprintf(szPrtBuf, "%s%s", szPadData, szPrintLineData);

		inDISP_LogPrintf(szPrtBuf);
		inPrintLineCnt++;
	};

	return (VS_SUCCESS);
}

/*
Function	:inDISP_Wait
Date&Time	:2015/6/7 下午 8:28
Describe	:等待時間(毫秒)
*/
int inDISP_Wait(int inmSecond)
{
	CTOS_Delay(inmSecond);
	return (VS_SUCCESS);
}

/*
Function	:inDISP_BEEP
Date&Time	:2015/7/31 下午 4:37
Describe	:BEEP聲（次數）
*/
int inDISP_BEEP(int inCount, int inmSecond)
{
	int i;

	for (i = 0; i < inCount; i++)
	{
		CTOS_Beep();
		inDISP_Wait(inmSecond);
	}

	return (VS_SUCCESS);
}

/*
Function        :inDISP_Sound
Date&Time       :2017/4/28 下午 1:47
Describe        :可調音的Beep?
 *		冷知識：
 *		BPM=60時 代表一分鐘有60個4音符，則每個四分音符占1 min/60 = 1second
 *		頻率
 *		DO: 261.6
 *		Re: 293.7
 *		MI: 329.6
 *		Fa: 349.2
 *		So: 392.0
 *		La: 440
 *		Si: 493.9
 *		高音Do : 523.3
*/
int inDISP_Sound(unsigned short usFreq, unsigned short usDuration)
{
	if (CTOS_Sound(usFreq, usDuration) != d_OK)
		return (VS_ERROR);

	return (VS_SUCCESS);
}

/*
Function	:inDISP_TimeoutStart
Date&Time	:2017/11/16 下午 1:17
Describe	:紀錄倒數時間 使用記數為 _TIMER_NEXSYS_4_
*/
int inDISP_TimeoutStart(int inTimeout)
{
	gulDispTimeStart = ulFunc_CalculateRunTime_Start();
	ginDispTimeOut = inTimeout;

	inDISP_Timer_Start(_TIMER_NEXSYS_4_, 1);

	return (VS_SUCCESS);
}

/*
Function	:inDISP_TimeoutCheck
Date&Time	:2017/11/16 下午 1:17
Describe	:CTLS Check是否TimeOut
*/
int inDISP_TimeoutCheck(int inFontSize, int inLine, int inPlace)
{
	int inRemainSecond = 0;
	char szTemplate[10 + 1];
	unsigned long ulSecond = 0;
	unsigned long ulMilliSecond = 0;

	inFunc_GetRunTime(gulDispTimeStart, &ulSecond, &ulMilliSecond);
	inRemainSecond = ginDispTimeOut - ulSecond;

	/* 檢查TIMEOUT，至少一秒才顯示差別 */
	if (inTimerGet(_TIMER_NEXSYS_4_) == VS_SUCCESS)
	{
		/* 清空 */
		inDISP_EnglishFont_Color("   ", inFontSize, inLine, _COLOR_RED_, inPlace);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d", inRemainSecond);
		inDISP_EnglishFont_Color(szTemplate, inFontSize, inLine, _COLOR_RED_, inPlace);

		/*  [國泰車麻吉] 用來判斷是否要再接收ECR的資料，用在讀卡時的第二段交易 2022/2/18 [SAM] */
		if (inRemainSecond < 5)
			inSetStopEcrReceive(VS_FALSE);
		else
			inSetStopEcrReceive(VS_TRUE);

		inDISP_Timer_Start(_TIMER_NEXSYS_4_, 1);
	}

	/* 0秒時為timeout */
	if (inRemainSecond <= 0)
		return (VS_TIMEOUT);
	else
		return (VS_SUCCESS);
}

/*
Function        :inDISP_BackLight_Set
Date&Time       :2018/3/13 下午 1:33
Describe        :
*/
int inDISP_BackLight_Set(unsigned char uszLED, unsigned char uszOnOff)
{
	char szDebugMsg[100 + 1] = {0};
	unsigned short usRetVal = VS_ERROR;

	usRetVal = CTOS_BackLightSet(uszLED, uszOnOff);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BackLightSet Err :0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BackLightSet() OK");
			inDISP_LogPrintf(szDebugMsg);
		}
	}

	return (VS_SUCCESS);
}

/*
Function	:inDISP_LogPrintfWithFlag
Date&Time	:  2018/10/11
Describe	:Debug inDISP_LogPrintfWithFlag
*/
int inDISP_LogPrintfWithFlag(char *szStr, ...)
{
	char szTemplate[20 + 1] = {0};
	char szDebugMsg[5000 + 1] = {0};
	RTC_NEXSYS srRTC = {0};
	va_list list; /* 加入不定參數作法 2018/5/29 上午 11:06 */
	if (ginDebug == VS_TRUE)
	{
		/* 加入測試訊息時間 2018/3/13 上午 10:19 */
		inFunc_GetSystemDateAndTime(&srRTC);
		sprintf(szTemplate, "%02d-%02d-%02d %02d:%02d:%02d ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		printf(szTemplate);

		va_start(list, szStr);
		vsprintf(szDebugMsg, szStr, list);
		va_end(list);
		printf(szDebugMsg);
		printf("\n");
	}
	return (VS_SUCCESS);
}

/*
Function	:inDISP_LogPrintfWithFlag
Date&Time	:  2018/10/11
Describe	:Debug inDISP_LogPrintfWithFlagForTimeTest
*/
int inDISP_LogPrintfWithFlagForTimeTest(char *szStr, ...)
{
	char szTemplate[20 + 1] = {0};
	char szDebugMsg[5000 + 1] = {0};
	RTC_NEXSYS srRTC = {0};
	va_list list; /* 加入不定參數作法 2018/5/29 上午 11:06 */
	if (ginTimeLogDebug == VS_TRUE)
	{
		/* 加入測試訊息時間 2018/3/13 上午 10:19 */
		inFunc_GetSystemDateAndTime(&srRTC);
		sprintf(szTemplate, "%02d-%02d-%02d %02d:%02d:%02d ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		printf(szTemplate);

		va_start(list, szStr);
		vsprintf(szDebugMsg, szStr, list);
		va_end(list);
		printf(szDebugMsg);
		printf("\n");
	}
	return (VS_SUCCESS);
}

/*
Function	 : inDISP_DispLogAndWriteFlie
Date&Time  : 2018/10/11
Describe	 : 統一顯示及寫入檔案用，顯示還是需看全域變數 ginDebug 的控制
*/
int inDISP_DispLogAndWriteFlie(char *szStr, ...)
{
#if 1
	char szTemplate[25 + 1] = {0};
	char szDebugMsg[2048 + 1] = {0};
	RTC_NEXSYS srRTC = {0};
	va_list list;

	//	/* 用區域的時間計算 */
	//	unsigned long ulRunTime;
	//	inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "DISP_5400 Write File INIT" );

	/* 加入測試訊息時間 2018/3/13 上午 10:19 */
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szTemplate, "%02d-%02d-%02d %02d:%02d:%02d F ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

	va_start(list, szStr);
	vsprintf(szDebugMsg, szStr, list);
	va_end(list);

	inFILE_LOG_WriteLog(szTemplate, szDebugMsg);

	if (ginDebug == VS_TRUE)
	{
		printf(szTemplate);
		printf(szDebugMsg);
		printf("\n");
	}

//	inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "DISP_5400 Write File END");
#endif
	return (VS_SUCCESS);
}

/*
Function	 : inDISP_LockEdcDispLogAndWriteFlie
Date&Time  : 2018/10/11
Describe	 : 統一顯示及寫入檔案用，顯示還是需看全域變數 ginDebug 的控制
*/
int inDISP_LockEdcDispLogAndWriteFlie(char *szStr, ...)
{
	char szTemplate[25 + 1] = {0};
	char szDebugMsg[5000 + 1] = {0};
	RTC_NEXSYS srRTC = {0};
	va_list list;

	/* 加入測試訊息時間 2018/3/13 上午 10:19 */
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szTemplate, "%02d-%02d-%02d %02d:%02d:%02d LF ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

	va_start(list, szStr);
	vsprintf(szDebugMsg, szStr, list);
	va_end(list);

	inFILE_LOG_WriteLog(szTemplate, szDebugMsg);

	if (ginDebug == VS_TRUE)
	{
		printf(szTemplate);
		printf(szDebugMsg);
		printf("\n");
	}

	inFunc_EDCLock();

	return (VS_SUCCESS);
}

/*
Function	:inDISP_TrunOnOffLightOnPanel
Date&Time	:  2018/10/11
Describe	: 開關 UPT1000 面版上磁條與晶片的燈
*/
int inDISP_TrunOnOffLightOnPanel(unsigned char *srLightName, int inOpation)
{
	int i = 0;

	while (1)
	{
		inDISP_LogPrintfWithFlag("[%d] [%x] ", i, srLightName[i]);
		if (srLightName[i] == 0xFF)
			break;

		if (inOpation == _LIGHT_ON_)
		{
			CTOS_Beep();
			CTOS_LEDSet(srLightName[i], d_ON);
		}
		else
			CTOS_LEDSet(srLightName[i], d_OFF);
		i++;
	}

	return VS_SUCCESS;
}

/*
Function	 : inDISP_LogPrintf
Date&Time : 2022/6/1 下午 2:16
Describe	 : Debug Printf
*/
int inLogPrintf(char *szlocation, char *szStr, ...)
{
	char szTemplate[20 + 1] = {0};
	char szDebugMsg[5000 + 1] = {0};
	unsigned long ulMileSecond = 0;
	RTC_NEXSYS srRTC = {0};
	va_list list; /* 加入不定參數作法 2018/5/29 上午 11:06 */

	/* 加入測試訊息時間 2018/3/13 上午 10:19 */
	inFunc_GetSystemDateAndTime(&srRTC);
	ulMileSecond = ((CTOS_TickGet() - gulTickCorrection) % 100);
	sprintf(szTemplate, "%02d-%02d-%02d %02d:%02d:%02d.%02lu ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, ulMileSecond);
	printf(szTemplate);

	/* 加入FILE和行數定位 */
	printf("%s", szlocation);

	va_start(list, szStr);
	vsprintf(szDebugMsg, szStr, list);
	va_end(list);
	printf(szDebugMsg);
	printf("\n");

	return (VS_SUCCESS);
}

/*
Function        :inDISP_RTC_Tick_Correction
Date&Time       :2019/6/26 上午 10:47
Describe        :
 * 新增電票 log使用方法 2022/6/7  [SAM]
*/
int inDISP_RTC_Tick_Correction(void)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inDISP_RTC_Tick_Correction() START !");
	}

	gulTickCorrection = CTOS_TickGet() % 100;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "gulTickCorrection:%lu", gulTickCorrection);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inDISP_RTC_Tick_Correction() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

// TODO: 看要不要改名,目的是在顯示資料, DEBUG 用
void PrintHexToStringFix(unsigned char *Input, int InputLen)
{
	int i = 0, j = 0;
	char Debugbuf[5000];

	if (InputLen <= 500)
	{
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (i = 0; i < InputLen; i++)
		{
			sprintf(&Debugbuf[i * 2], "%02X", *(Input + i));
		}
		inDISP_LogPrintfAt(AT, "Debugbuf:[ %s ]", &Debugbuf);
	}
	else if (InputLen <= 1000)
	{
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (i = 0; i <= 500; i++)
		{
			sprintf(&Debugbuf[i * 2], "%02X", *(Input + i));
		}
		inDISP_LogPrintfAt(AT, "Debugbuf:[ %s ]", &Debugbuf);

		j = 0;
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (; i < InputLen; i++)
		{
			// sprintf(&Debugbuf[i * 2], "%02X", *(Input + i));
			sprintf(&Debugbuf[j * 2], "%02X", *(Input + i));
			j++;
		}
		inDISP_LogPrintfAt(AT, "Debugbuf2:[ %s ]", &Debugbuf);
	}
	else if (InputLen <= 2000 && InputLen > 1000)
	{
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (i = 0; i <= 500; i++)
		{
			sprintf(&Debugbuf[i * 2], "%02X", *(Input + i));
		}
		inDISP_LogPrintfAt(AT, "Debugbuf:[ %s ]", &Debugbuf);

		j = 0;
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (; i <= 1000; i++)
		{
			sprintf(&Debugbuf[j * 2], "%02X", *(Input + i));
			j++;
		}
		inDISP_LogPrintfAt(AT, "Debugbuf2:[ %s ]", &Debugbuf);

		j = 0;
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (; i <= 1500; i++)
		{
			sprintf(&Debugbuf[j * 2], "%02X", *(Input + i));
			j++;
		}
		inDISP_LogPrintfAt(AT, "Debugbuf3:[ %s ]", &Debugbuf);

		j = 0;
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (; i <= InputLen; i++)
		{
			sprintf(&Debugbuf[j * 2], "%02X", *(Input + i));
			j++;
		}
		inDISP_LogPrintfAt(AT, "Debugbuf4:[ %s ]", &Debugbuf);
	}
	else if (InputLen <= 3000 && InputLen > 2000)
	{
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (i = 0; i <= 500; i++)
		{
			sprintf(&Debugbuf[i * 2], "%02X", *(Input + i));
		}
		inDISP_LogPrintfAt(AT, "Debugbuf:[ %s ]", &Debugbuf);

		j = 0;
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (; i <= 1000; i++)
		{
			sprintf(&Debugbuf[j * 2], "%02X", *(Input + i));
			j++;
		}
		inDISP_LogPrintfAt(AT, "Debugbuf2:[ %s ]", &Debugbuf);

		j = 0;
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (; i <= 1500; i++)
		{
			sprintf(&Debugbuf[j * 2], "%02X", *(Input + i));
			j++;
		}
		inDISP_LogPrintfAt(AT, "Debugbuf3:[ %s ]", &Debugbuf);

		j = 0;
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (; i <= 2000; i++)
		{
			sprintf(&Debugbuf[j * 2], "%02X", *(Input + i));
			j++;
		}
		inDISP_LogPrintfAt(AT, "Debugbuf4:[ %s ]", &Debugbuf);

		j = 0;
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (; i <= 2500; i++)
		{
			sprintf(&Debugbuf[j * 2], "%02X", *(Input + i));
			j++;
		}
		inDISP_LogPrintfAt(AT, "Debugbuf5:[ %s ]", &Debugbuf);

		j = 0;
		memset(Debugbuf, 0x00, sizeof(Debugbuf));
		for (; i <= InputLen; i++)
		{
			sprintf(&Debugbuf[j * 2], "%02X", *(Input + i));
			j++;
		}
		inDISP_LogPrintfAt(AT, "Debugbuf6:[ %s ]", &Debugbuf);
	}
	else
	{
		inDISP_LogPrintfAt(AT, "not write!");
	}
}

/*
Function	:inDISP_LogPrintfAt
Date&Time: 2022/6/10 下午 2:44
Describe	:Debug Printf
*/
int inDISP_LogPrintfAt(char *szlocation, char *szStr, ...)
{
	char szTemplate[50 + 1] = {0}; /*20220119,浩瑋標記fix*/
	char szDebugMsg[5000 + 1] = {0};
	unsigned long ulMileSecond = 0;
	RTC_NEXSYS srRTC = {0};
	va_list list; /* 加入不定參數作法 2018/5/29 上午 11:06 */

	memset(szTemplate, 0x00, sizeof(szTemplate)); /*20220119,浩瑋標記fix*/
	/* 加入測試訊息時間 2018/3/13 上午 10:19 */
	inFunc_GetSystemDateAndTime(&srRTC);
	ulMileSecond = ((CTOS_TickGet() - gulTickCorrection) % 100);
	sprintf(szTemplate, "%02d-%02d-%02d %02d:%02d:%02d.%02lu ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, ulMileSecond);
	printf(szTemplate);

	/* 加入FILE和行數定位 */
	printf("%s", szlocation);

	va_start(list, szStr);
	vsprintf(szDebugMsg, szStr, list);
	va_end(list);
	printf(szDebugMsg);
	printf("\n");

	return (VS_SUCCESS);
}

/*
Function	: inDISP_SetMultiReceiveTimeout
Date&Time	: 2022/11/29 上午 9:29
Describe	:
 *  [外接設備設定] 新增
*/
int inDISP_SetMultiReceiveTimeout()
{
	int inRemainSecond = 0;
	unsigned long ulSecond = 0;
	unsigned long ulMilliSecond = 0;

	inFunc_GetRunTime(gulDispTimeStart, &ulSecond, &ulMilliSecond);
	inRemainSecond = ginDispTimeOut - ulSecond;
	/* 少於3秒就不進行 ECR 接收 */
	if (inRemainSecond <= 2)
	{
		inDISP_LogPrintfWithFlag(" SetMultiReceiveTimeout [%d]  *Warning* Line[%d]", inRemainSecond, __LINE__);
		return VS_TIMEOUT;
	}
	else
	{
		gstMultiOb.srSetting.inTimeout = inRemainSecond;
		return VS_SUCCESS;
	}
}

/**
 * RESPONSE_V3
 * [新增SVC功能]  [SAM]
 * DEBUG使用，顯示對應的定義用
 */
char *pszDisp_GetResponseV3Message(int inRespCode)
{
	memset(st_szMsgTemp, 0x00, sizeof(st_szMsgTemp));

	if (inRespCode == VS_USER_CANCEL)
		strcat(st_szMsgTemp, "VS_USER_CANCEL");
	else if (inRespCode == VS_WAVE_INVALID_SCHEME_ERR)
		strcat(st_szMsgTemp, "VS_WAVE_INVALID_SCHEME_ERR");
	else if (inRespCode == VS_WAVE_AMOUNT_ERR)
		strcat(st_szMsgTemp, "VS_WAVE_AMOUNT_ERR");
	else if (inRespCode == VS_WAVE_ERROR)
		strcat(st_szMsgTemp, "VS_WAVE_ERROR");
	else if (inRespCode == VS_NO_CARD_BIN)
		strcat(st_szMsgTemp, "VS_NO_CARD_BIN");
	else if (inRespCode == VS_CARD_EXP_ERR)
		strcat(st_szMsgTemp, "VS_CARD_EXP_ERR");
	else if (inRespCode == VS_LAST_PAGE)
		strcat(st_szMsgTemp, "VS_LAST_PAGE");
	else if (inRespCode == VS_PREVIOUS_PAGE)
		strcat(st_szMsgTemp, "VS_PREVIOUS_PAGE");
	else if (inRespCode == VS_NEXT_PAGE)
		strcat(st_szMsgTemp, "VS_NEXT_PAGE");
	else if (inRespCode == VS_FUNC_CLOSE_ERR)
		strcat(st_szMsgTemp, "VS_FUNC_CLOSE_ERR");
	else if (inRespCode == VS_HG_REWARD_COMM_ERR)
		strcat(st_szMsgTemp, "VS_HG_REWARD_COMM_ERR");
	else if (inRespCode == VS_PRINTER_OVER_HEAT)
		strcat(st_szMsgTemp, "VS_PRINTER_OVER_HEAT");
	else if (inRespCode == VS_PRINTER_PAPER_OUT)
		strcat(st_szMsgTemp, "VS_PRINTER_PAPER_OUT");
	else if (inRespCode == VS_CALLBANK)
		strcat(st_szMsgTemp, "VS_CALLBANK");
	else if (inRespCode == VS_ISO_PACK_ERR)
		strcat(st_szMsgTemp, "VS_ISO_PACK_ERR");
	else if (inRespCode == VS_ISO_UNPACK_ERROR)
		strcat(st_szMsgTemp, "VS_ISO_UNPACK_ERROR");
	else if (inRespCode == VS_ICC_INSERT_ERROR)
		strcat(st_szMsgTemp, "VS_ICC_INSERT_ERROR");
	else if (inRespCode == VS_SWIPE_ERROR)
		strcat(st_szMsgTemp, "VS_SWIPE_ERROR");
	else if (inRespCode == VS_FILE_ERROR)
		strcat(st_szMsgTemp, "VS_FILE_ERROR");
	else if (inRespCode == VS_READ_ERROR)
		strcat(st_szMsgTemp, "VS_READ_ERROR");
	else if (inRespCode == VS_WRITE_ERROR)
		strcat(st_szMsgTemp, "VS_WRITE_ERROR");
	else if (inRespCode == VS_USER_OPER_ERR)
		strcat(st_szMsgTemp, "VS_USER_OPER_ERR");
	else if (inRespCode == VS_CLOSE_ERROR)
		strcat(st_szMsgTemp, "VS_CLOSE_ERROR");
	else if (inRespCode == VS_ABORT)
		strcat(st_szMsgTemp, "VS_ABORT");
	else if (inRespCode == VS_EMV_CARD_OUT)
		strcat(st_szMsgTemp, "VS_EMV_CARD_OUT");
	else if (inRespCode == VS_WAVE_NO_DATA)
		strcat(st_szMsgTemp, "VS_WAVE_NO_DATA");
	else
		sprintf(st_szMsgTemp, "%d Not Exist", inRespCode);

	return &st_szMsgTemp;
}
