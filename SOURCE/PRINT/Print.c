#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_qrcode.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Transaction.h"
#include "../FUNCTION/FILE_FUNC/File.h"
#include "../FUNCTION/FILE_FUNC/FIleLogFunc.h"
#include "../FUNCTION/Function.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DispMsg.h"
#include "../EVENT/MenuMsg.h"
#include "Print.h"
#include "PrtMsg.h"


int			ginSetPRFFont = -1;		/* 判斷目前SetFont */
int			ginSetFontSize = -1;		/* 判斷目前SetFont */
BMPHeight	gsrBMPHeight;
extern  int		ginDebug;			/* Debug使用 extern */
extern	int	ginISODebug;
extern	int	ginMachineType;
DISPLAY_OBJECT	srDispObj;			/* Error Display使用 */

BufferArrangeUnderLine	gsrBAUL[5];
int			ginBAUL_Index = 0;

/*
Function	:inPRINT_Initial
Date&Time	:2015/6/8 上午 10:24
Describe	:列印參數初始化
*/
int inPRINT_Initial(void)
{
	char		szTemplate[44 + 1];
	unsigned short	usRetVal;

	if (inFunc_Check_Print_Capability(ginMachineType) == VS_SUCCESS)
	{

		usRetVal = CTOS_PrinterSetHeatLevel(0); /* 設定字體顏色深淺 0~6 淺到深 */

		if (usRetVal != d_OK)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "代碼：0x%04X", usRetVal);	/* 錯誤代碼 */

			/* 印表機初始化錯誤 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_PRINT_INIT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);

			return (VS_ERROR);
		}

		usRetVal = CTOS_PrinterFontSelectMode(d_FONT_TTF_MODE);

		if (usRetVal != d_OK)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "代碼：0x%04X", usRetVal);       /* 錯誤代碼 */

			/* 印表機初始化錯誤 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_PRINT_INIT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);

			return (VS_ERROR);
		}
	}

	return (VS_SUCCESS);
}

/*
Function	:inPRINT_TTF_SetFont
Date&Time	:2015/6/8 上午 10:24
Describe	:語言選擇列印的TTF
*/
int inPRINT_TTF_SetFont(int inLanguage, int inFontStyle)
{
	char		szTemplate[44 + 1];
        unsigned short	usRetVal = 0;

	if (inLanguage == _PRT_CHINESE_1_)
        {
		usRetVal = CTOS_PrinterTTFSelect((BYTE *)_PRT_CHINESE_FONE_1_, inFontStyle);	/* 設定TTF檔 微軟正黑體 0-regular 1-Italic 2-Bold 3-Bold italic*/
                ginSetPRFFont = _PRT_CHINESE_1_;
        }
	else if (inLanguage == _PRT_CHINESE_2_)
        {
		usRetVal = CTOS_PrinterTTFSelect((BYTE *)_PRT_CHINESE_FONE_2_ , inFontStyle);	/* 設定TTF檔 COURBD 0-regular 1-Italic 2-Bold 3-Bold italic*/
                ginSetPRFFont = _PRT_CHINESE_2_;
        }
//	else if (inLanguage == _PRT_ENGLISH_)
//        {
//		usRetVal = CTOS_PrinterTTFSelect((BYTE *)_PRT_ENGLISH_FONE_1_ , inFontStyle);	/* 設定TTF檔 COURBD 0-regular 1-Italic 2-Bold 3-Bold italic*/
//                ginSetPRFFont = _PRT_ENGLISH_;
//        }

        if (usRetVal != d_OK)
        {
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szTemplate, "代碼：0x%04X", usRetVal);       /* 錯誤代碼 */
		
		/* 印表機TTF錯誤 */
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_Msg_BMP(_ERR_PRINT_TTF_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
		
                return (VS_ERROR);
        }

	return (VS_SUCCESS);
}

/*
Function	:inPRINT_Space
Date&Time	:2015/6/8 上午 10:24
Describe	:列印空白行
*/
int inPRINT_SpaceLine(int inLine)
{
	char		szTemplate[44 + 1];
	unsigned char	uszKey;
	unsigned short	usRetVal;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		usRetVal = CTOS_PrinterFline(d_LINE_DOT * inLine);

		if (usRetVal == d_OK)
		{
			return (VS_SUCCESS);
		}
		else
		{
			if (usRetVal == d_PRINTER_PAPER_OUT)
			{
				/* 印表機缺紙請裝紙 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_ERR_PRINT_PAPER_OUT_, 0, _COORDINATE_Y_LINE_8_6_);
				inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);

				while (1)
				{
					inDISP_BEEP(1, 1000);
					uszKey = uszKBD_Key();

					if (uszKey == _KEY_ENTER_)
					{
						break;
					}
					else
					{
						continue;
					}
				}

				return (VS_PRINTER_PAPER_OUT);
			}
			else if (usRetVal == d_PRINTER_HEAD_OVERHEAT)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));	
				sprintf(szTemplate, "印表機過熱");	/* 錯誤代碼 */

				/* 印表機過熱 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);

				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf(szTemplate);
					inDISP_LogPrintf("inPRINT_Buffer_OutPut() END!");
				}

				return (VS_PRINTER_OVER_HEAT);
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));	
				sprintf(szTemplate, "代碼：0x%04X", usRetVal);	/* 錯誤代碼 */

				/* 印表機錯誤 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
			}

			return (VS_ERROR);
		}
	}
}

/*
Function	:inPRINT_Put_Graphic
Date&Time	:2015/6/8 上午 10:24
Describe	:列印圖片
*/
int inPRINT_PutGraphic(unsigned char *uszFilename)
{
	char		szTemplate[44 + 1];
	unsigned char	uszKey;
        unsigned short	usRetVal;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		usRetVal = CTOS_PrinterBMPPic(0, uszFilename);

		if (usRetVal == d_OK)
		{
			return (VS_SUCCESS);
		}
		else
		{
			if (usRetVal == d_PRINTER_PAPER_OUT)
			{
				/* 印表機缺紙請裝紙 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_ERR_PRINT_PAPER_OUT_, 0, _COORDINATE_Y_LINE_8_6_);
				inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);

				while (1)
				{
					inDISP_BEEP(1, 1000);
					uszKey = uszKBD_Key();

					if (uszKey == _KEY_ENTER_)
					{
						break;
					}
					else
					{
						continue;
					}
				}

				return (VS_PRINTER_PAPER_OUT);
			}
			else if (usRetVal == d_PRINTER_HEAD_OVERHEAT)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));	
				sprintf(szTemplate, "印表機過熱");	/* 錯誤代碼 */

				/* 印表機過熱 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);

				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf(szTemplate);
					inDISP_LogPrintf("inPRINT_Buffer_OutPut() END!");
				}

				return (VS_PRINTER_OVER_HEAT);
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));	
				sprintf(szTemplate, "代碼：0x%04X", usRetVal);	/* 錯誤代碼 */

				/* 印表機錯誤 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
			}

			return (VS_ERROR);
		}
	}
}

/*
Function	:inPRINT_ChineseFont
Date&Time	:2015/6/8 上午 10:24
Describe	:列印TTF中文字
*/
int inPRINT_ChineseFont(char *szStr, int inFontSize)
{
	char		szTemplate[44 + 1];
	unsigned char	uszKey;
	unsigned short	usRetVal;
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (ginSetPRFFont != _PRT_CHINESE_1_)
		{
			/* 判斷是否已經SetFont過_PRT_CHINESE_，如果沒有就要Set */
			inPRINT_TTF_SetFont(_PRT_CHINESE_1_, _FONT_REGULAR_);
		}

		if (ginSetFontSize != inFontSize)
		{
			CTOS_LanguagePrinterFontSize(inFontSize, 0, 0);
			ginSetFontSize = inFontSize;
		}

		usRetVal = CTOS_PrinterPutString((unsigned char *)szStr);

		if (usRetVal == d_OK)
		{
			return (VS_SUCCESS);
		}
		else
		{
			if (usRetVal == d_PRINTER_PAPER_OUT)
			{
				/* 印表機缺紙請裝紙 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_ERR_PRINT_PAPER_OUT_, 0, _COORDINATE_Y_LINE_8_6_);
				inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);

				while (1)
				{
					inDISP_BEEP(1, 1000);
					uszKey = uszKBD_Key();

					if (uszKey == _KEY_ENTER_)
					{
						break;
					}
					else
					{
						continue;
					}
				}

				return (VS_PRINTER_PAPER_OUT);
			}
			else if (usRetVal == d_PRINTER_HEAD_OVERHEAT)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));	
				sprintf(szTemplate, "印表機過熱");	/* 錯誤代碼 */

				/* 印表機過熱 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);

				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf(szTemplate);
					inDISP_LogPrintf("inPRINT_Buffer_OutPut() END!");
				}

				return (VS_PRINTER_OVER_HEAT);
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));	
				sprintf(szTemplate, "代碼：0x%04X", usRetVal);	/* 錯誤代碼 */

				/* 印表機錯誤 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
			}

			return (VS_ERROR);
		}
	}

}

/*
Function	:inPRINT_EnglishFont
Date&Time	:2015/6/8 上午 10:24
Describe	:列印TTF英文字
*/
int inPRINT_EnglishFont(char *szStr, int inFontSize)
{
	char		szTemplate[44 + 1];
	unsigned char	uszKey;
        unsigned short	usRetVal;
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{

		if (ginSetPRFFont != _PRT_ENGLISH_)
		{
			/* 判斷是否已經SetFont過_PRT_ENGLISH_，如果沒有就要Set */
			inPRINT_TTF_SetFont(_PRT_ENGLISH_, _FONT_REGULAR_);
		}

		CTOS_LanguagePrinterFontSize(inFontSize, 0, 0);
		usRetVal = CTOS_PrinterPutString((unsigned char*)szStr);

		if (usRetVal == d_OK)
		{
			return (VS_SUCCESS);
		}
		else
		{
			if (usRetVal == d_PRINTER_PAPER_OUT)
			{
				/* 印表機缺紙請裝紙 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_ERR_PRINT_PAPER_OUT_, 0, _COORDINATE_Y_LINE_8_6_);
				inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);

				while (1)
				{
					inDISP_BEEP(1, 1000);
					uszKey = uszKBD_Key();

					if (uszKey == _KEY_ENTER_)
					{
						break;
					}
					else
					{
						continue;
					}
				}

				return (VS_PRINTER_PAPER_OUT);
			}
			else if (usRetVal == d_PRINTER_HEAD_OVERHEAT)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));	
				sprintf(szTemplate, "印表機過熱");	/* 錯誤代碼 */

				/* 印表機過熱 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);

				if (ginDebug == VS_TRUE)
				{
					inDISP_LogPrintf(szTemplate);
					inDISP_LogPrintf("inPRINT_Buffer_OutPut() END!");
				}

				return (VS_PRINTER_OVER_HEAT);
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));	
				sprintf(szTemplate, "代碼：0x%04X", usRetVal);	/* 錯誤代碼 */

				/* 印表機錯誤 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
			}

			return (VS_ERROR);

		}
	}
}

/*
Function	:inPRINT_TwoQR
Date&Time	:2015/6/8 上午 10:24
Describe	:列印雙QR
*/
int inPRINT_TwoQR(char *szQR_1, char *szQR_2)
{
#ifndef	_LOAD_KEY_AP_

	char			szTemplate[44 + 1];
	unsigned char		uszOutputBuffer[384 * 8 * 20];
	unsigned short		usRetVal;
	CTOS_QRCODE_INFO	qrcodeInfo;
	

	usRetVal = CTOS_PrinterBufferInit(uszOutputBuffer, 8 * 20); /* Height of buffer in pixel. Must be byte-aligned (multiple of 8). */

	if(usRetVal != d_OK)
	{
                memset(szTemplate, 0x00, sizeof(szTemplate));	
		sprintf(szTemplate, "代碼：0x%04X", usRetVal);	/* 錯誤代碼 */

		/* 印表機初始化錯誤 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_Msg_BMP(_ERR_PRINT_INIT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
		
		return (VS_ERROR);
	}

	CTOS_PrinterSetHeatLevel(4); /* 設定字體顏色深淺 0~6 淺到深 */


	qrcodeInfo.InfoVersion = QR_INFO_VERSION;
	qrcodeInfo.Size = 3; /* 縮放 5預設值 10:兩倍 1:五分之一 */
	qrcodeInfo.Version = QR_VERSION45X45; /* QR Code的大小 21 x 21 到 177 x 177 財政部查電子發票 至少要V6以上 */
	qrcodeInfo.Level = QR_LEVEL_M; /* 錯誤修正(資料回復能力) Level L M Q H 商業用途: L or M 工業環境: Q or H 設定愈高影響可儲存的資料量*/

	usRetVal = CTOS_PrinterBufferSelectActiveAddress(uszOutputBuffer); /* 先不列印 儲存在Buffer */

	if(usRetVal != d_OK)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));	
		sprintf(szTemplate, "代碼：0x%04X", usRetVal);	/* 錯誤代碼 */

		/* 印表機錯誤 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
		
		return (VS_ERROR);
	}

	//CTOS_QRcodeBufferPrinter(CTOS_QRCODE_INFO *pInfo, const char *pMessageText, USHORT usXPos, USHORT usYPos);
	//Put QRcode into Buffer
	usRetVal = CTOS_QRCodePrinterBuffer(&qrcodeInfo, szQR_1, 25, 1);  /* X軸起始位置 25 Y軸起始位置 1 */
	usRetVal = CTOS_QRCodePrinterBuffer(&qrcodeInfo, szQR_2, 225, 1);  /* X軸起始位置 225 Y軸起始位置 1 */
	usRetVal = CTOS_PrinterBufferOutput(uszOutputBuffer, 20);   /* 印出Buffer */

	if (usRetVal != d_OK)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));	
		sprintf(szTemplate, "代碼：0x%04X", usRetVal);	/* 錯誤代碼 */

		/* 印表機錯誤 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
		
		return (VS_ERROR);
	}

#endif
	return (VS_SUCCESS);
}

/*
Function	:inPRINT_Barcode
Date&Time	:2015/6/8 上午 10:24
Describe	:列印Barcode
*/
int inPRINT_Barcode(char *szBarcode)
{
	char		szTemplate[44 + 1];
	unsigned char	uszKey;
        unsigned short	usRetVal;

	usRetVal = CTOS_PrinterCode39Barcode(1,1,(BYTE *)szBarcode,strlen(szBarcode),2,3,FALSE); /* TRUE 顯示裡面的字 FLASE 不顯示 */

        if (usRetVal == d_OK)
        {
                return (VS_SUCCESS);
        }
        else
	{
                if (usRetVal == d_PRINTER_PAPER_OUT)
                {
			/* 印表機缺紙請裝紙 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ERR_PRINT_PAPER_OUT_, 0, _COORDINATE_Y_LINE_8_6_);
			inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
			
			while (1)
			{
				inDISP_BEEP(1, 1000);
				uszKey = uszKBD_Key();

				if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else
				{
					continue;
				}
			}
			
			return (VS_PRINTER_PAPER_OUT);
                }
		else if (usRetVal == d_PRINTER_HEAD_OVERHEAT)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));	
                        sprintf(szTemplate, "印表機過熱");	/* 錯誤代碼 */
			
			/* 印表機過熱 */
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
			
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf(szTemplate);
				inDISP_LogPrintf("inPRINT_Buffer_OutPut() END!");
			}
			
			return (VS_PRINTER_OVER_HEAT);
		}
                else
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));	
                        sprintf(szTemplate, "代碼：0x%04X", usRetVal);	/* 錯誤代碼 */
			
			/* 印表機錯誤 */
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
                }

                return (VS_ERROR);

        }
}

/*
Function		:inPRINT_Buffer_Initial
Date&Time	:2016/2/18 上午 11:03
Describe		:初始化用到的各項結構
 *uszBuffer:	用來列印的Buffer
 *inYLength	高度有多少像素單位(8像素為一單位)
 *srFont_Attrib 字的大小，字與字之間的間距
 *srBhandle	用來管理buffer現在寫到哪裡，是否寫超過紙張或Buffer最底部
*/
int inPRINT_Buffer_Initial(unsigned char *uszBuffer, int inYLength, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0x00;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inPRINT_Buffer_Initial() START!");
	}
	
	memset(uszBuffer, 0x00, PB_CANVAS_X_SIZE * 8 * inYLength);
	memset(srFont_Attrib, 0x00, sizeof(FONT_ATTRIB));
	memset(srBhandle, 0x00, sizeof(BufferHandle));
	
	srFont_Attrib->X_Zoom = 1;	/* 1, it means normal size, and 2 means double size. 0 means print nothing . */
	srFont_Attrib->Y_Zoom = 1;	/* 1, it means normal size, and 2 means double size. 0 means print nothing . */
	srFont_Attrib->X_Space = 0;	/* The space in dot to insert between each character in x coordinate. */
	srFont_Attrib->Y_Space = 0;	/* The space in dot to insert between each character in y coordinate. */
	srFont_Attrib->FontSize = _PRT_HEIGHT_;
	
	srBhandle->inXcurrent = 0;
	srBhandle->inYcurrent = 0;
	srBhandle->inYcurrentMAX = 0;
	srBhandle->inYcover = 0;
	srBhandle->inXbound = PB_CANVAS_X_SIZE;	/* 紙張最寬到哪裡 */
	srBhandle->inYbound = 8 * inYLength;	/* buffer最底部宣告到哪裡 */
	
	/* 這隻是DefaultBuffer用的Initial，QRcode、Barcode會用到，一定要call */
	CTOS_PrinterBufferEnable();
	
	/* 這隻是字串用的Initial */
	usRetVal = CTOS_PrinterBufferInit(uszBuffer, srBhandle->inYbound);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PrinterBufferInit Err usRetVal: 0x%04X", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	CTOS_PrinterBufferSelectActiveAddress(uszBuffer);
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inPRINT_Buffer_Initial() END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inPRINT_Buffer_PutIn
Date&Time	:2016/2/18 上午 9:54
Describe	:把string put in到Buffer中，目前設計同一行只能put in 多次且能放不同大小的字並選擇靠左置中或靠右（要自行控制同一行內字是否重疊）
 *szString:		put in 的字串
 *inFontSize:	put in的字型大小	
 *uszBuffer:		用來列印的Buffer
 *srBhandle:		用來管理buffer現在寫到哪裡，是否寫超過紙張或Buffer最底部
 *srFont_Attrib:	字的大小，字與字之間的間距
 *inNextLine:	下次在Put in會在這一行還是下一行，放_LAST_ENTRY_表示下一次put in會放下一行
 *uszPrintPosition:有靠左、置中、靠右三種選擇(_PRINT_LEFT_、_PRINT_CENTER_、_PRINT_RIGHT_）
 * 
 * inYLength:	高度有多少像素單位(8像素為一單位)
 *
*/
int inPRINT_Buffer_PutIn(char* szString, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, unsigned char uszPrintPosition)
{
	int			inRetVal;		/* 若回傳沒紙要重印 */
	
	/* 設定列印字型大小 */
	srFont_Attrib->FontSize = inFontSize;
			
	/* 放超過Buffer高度，回傳錯誤(直接假設該行印最大字型，若會超出bound就直接印出來) */
	if ((srBhandle->inYcurrent + _MAX_Y_LENGTH_) > srBhandle->inYbound)
	{
		
		/* 當放超過高度 直接OutPut並清空Buffer(保險起見) */
		do
		{
			inRetVal = inPRINT_Buffer_OutPut(uszBuffer, srBhandle);
			
		} while (inRetVal != VS_SUCCESS);
			
		memset(uszBuffer, 0x00, srBhandle->inXbound * srBhandle->inYbound);

		srBhandle->inXcurrent = 0;
		srBhandle->inYcurrent = 0;
		srBhandle->inYcurrentMAX = 0;
	}
	
//	if (strlen(szString)_PRT_ISO_ && inNextLine == _LAST_ENTRY_)
//	{
//		/* 對特定字型超過一行會印到下一行 */
//		char	szPart1[38 + 1];
//		
//		memset(szPart1, 0x00, sizeof(szPart1));
//		memcpy(szPart1, szString, 38);
//		
//		inRetVal = inPRINT_Buffer_Sync_UnderLine(szPart1, uszBuffer, srFont_Attrib, srBhandle, inNextLine, uszPrintPosition, 0);
//		inRetVal = inPRINT_Buffer_Sync_UnderLine(&szString[38], uszBuffer, srFont_Attrib, srBhandle, inNextLine, uszPrintPosition, 0);
//	}
//	else
	{
		inRetVal = inPRINT_Buffer_Sync_UnderLine(szString, uszBuffer, srFont_Attrib, srBhandle, inNextLine, uszPrintPosition, 0);
	}
	
	return (inRetVal);	
}

/*
Function	:inPRINT_Buffer_PutIn_Specific_X_Position
Date&Time	:2016/9/9 下午 1:46
Describe	:此function說明同inPRINT_Buffer_PutIn，但可強制控制x軸位置，以應付要對齊某一座標點的時候
 *szString:		put in 的字串
 *inFontSize:		put in的字型大小	
 *uszBuffer:		用來列印的Buffer
 *srBhandle:		用來管理buffer現在寫到哪裡，是否寫超過紙張或Buffer最底部
 *srFont_Attrib:	字的大小，字與字之間的間距
 *inNextLine:		下次在Put in會在這一行還是下一行，放_LAST_ENTRY_表示下一次put in會放下一行
 *intXPosition:		x軸位置
 *
*/
int inPRINT_Buffer_PutIn_Specific_X_Position(char* szString, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, int intXPosition)
{
	int			inRetVal;		/* 若回傳沒紙要重印 */
		
	/* 設定列印字型大小 */
	srFont_Attrib->FontSize = inFontSize;
			
	/* 放超過Buffer高度，回傳錯誤(直接假設該行印最大字型，若會超出bound就直接印出來) */
	if ((srBhandle->inYcurrent + _MAX_Y_LENGTH_) > srBhandle->inYbound)
	{
		
		/* 當放超過高度 直接OutPut並清空Buffer(保險起見) */
		do
		{
			inRetVal = inPRINT_Buffer_OutPut(uszBuffer, srBhandle);
			
		} while (inRetVal != VS_SUCCESS);
			
		memset(uszBuffer, 0x00, srBhandle->inXbound * srBhandle->inYbound);

		srBhandle->inXcurrent = 0;
		srBhandle->inYcurrent = 0;
		srBhandle->inYcurrentMAX = 0;
	}
	
	inRetVal = inPRINT_Buffer_Sync_UnderLine(szString, uszBuffer, srFont_Attrib, srBhandle, inNextLine, _PRINT_BY_X_, intXPosition);
	
	return (inRetVal);
}

/*
Function	:inPRINT_Buffer_PutIn_Format_Auto_Change_Line
Date&Time	:2017/2/15 下午 5:05
Describe	:For 優惠兌換自動換行訊息，一律換行，inNextLine無作用，不知道會不會有Bug，建議少用
 *szString:		put in 的字串
 *inFontSize:		put in的字型大小	
 *uszBuffer:		用來列印的Buffer
 *srBhandle:		用來管理buffer現在寫到哪裡，是否寫超過紙張或Buffer最底部
 *srFont_Attrib:	字的大小，字與字之間的間距
 *inNextLine:		下次在Put in會在這一行還是下一行，放_LAST_ENTRY_表示下一次put in會放下一行
 *uszPrintPosition:	有靠左、置中、靠右三種選擇(_PRINT_LEFT_、_PRINT_CENTER_、_PRINT_RIGHT_）
 * 
 * inYLength:		高度有多少像素單位(8像素為一單位)
 *
*/
int inPRINT_Buffer_PutIn_Format_Auto_Change_Line(char* szString, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, unsigned char uszPrintPosition)
{
	int	inRetVal;		/* 若回傳沒紙要重印 */
	int	inFontXLen = 0;		/* 該大小每個字x寬度*/
	int	inLineLen = 0;
	int	inStart = 0;
	int	inCurrentLen = 0;
	int	inTotalLen;
	int	inOffset;
	int	inLineCnt = 0;		/* debug用 */
	char	szPart[200 + 1];
	BOOL	fChinese;
	
	/* 設定列印字型大小 */
	srFont_Attrib->FontSize = inFontSize;
			
	/* 放超過Buffer高度，回傳錯誤(直接假設該行印最大字型，若會超出bound就直接印出來) */
	if ((srBhandle->inYcurrent + _MAX_Y_LENGTH_) > srBhandle->inYbound)
	{
		
		/* 當放超過高度 直接OutPut並清空Buffer(保險起見) */
		do
		{
			inRetVal = inPRINT_Buffer_OutPut(uszBuffer, srBhandle);
			
		} while (inRetVal != VS_SUCCESS);
			
		memset(uszBuffer, 0x00, srBhandle->inXbound * srBhandle->inYbound);

		srBhandle->inXcurrent = 0;
		srBhandle->inYcurrent = 0;
		srBhandle->inYcurrentMAX = 0;
	}
	
	/* 總Byte數 */
	inTotalLen = strlen(szString);
	inFontXLen = srFont_Attrib->FontSize / 0x0100;
	
	do
	{
		/* 先判斷是否為Ascii Code  char > 127 會是負數 */
		if (szString[inCurrentLen] < 0)
		{
			fChinese = VS_TRUE;
			inLineLen += 2 * inFontXLen;
			inOffset = 3;
		}
		else
		{
			fChinese = VS_FALSE;
			inLineLen += inFontXLen;
			inOffset = 1;
		}
		
		/* 往前移動 */
		inCurrentLen += inOffset;
		
		/* PB_CANVAS_X_SIZE = 384 */
		if (inLineLen > PB_CANVAS_X_SIZE	||	/* 超過邊緣 */
		    inCurrentLen == inTotalLen)			/* 印完全部 */
		{
			/* 要超過邊緣了 先退回一格 */
			if (inLineLen > PB_CANVAS_X_SIZE)
			{
				inCurrentLen -= inOffset;
			}
			
			/* 行內長度歸0 */
			inLineLen = 0;
			
			memset(szPart, 0x00, sizeof(szPart));
			memcpy(szPart, &szString[inStart], inCurrentLen - inStart);
			
			inRetVal = inPRINT_Buffer_Sync_UnderLine(szPart, uszBuffer, srFont_Attrib, srBhandle, inNextLine, uszPrintPosition, 0);
			if (ginDebug == VS_TRUE)
			{
				char	szDebugMsg[100 + 1];
				
				inLineCnt ++;
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "這是第%d行",inLineCnt);
				inDISP_LogPrintf(szDebugMsg);
				inDISP_LogPrintf(szPart);
			}
			
			/* 起始點移動 */
			inStart = inCurrentLen;
		}
		else
		{
			
		}
			
	} while (inCurrentLen < inTotalLen);

	return (inRetVal);
}
/*
Function        :inPRINT_Buffer_Sync_UnderLine
Date&Time       :2016/9/9 下午 2:30
Describe        :這隻在Last_Entry出現前，會先把該行要PutIn收集起來，直到要到下一行時會找出該行最大的字型高度，如此一來，不管大字小字都會在同一底線
 *		目前可以最多PutIn五次(因為結構宣告大小為5，可自行增縮)
 * 
 *intXPosition:	若uszPrintPosition != _PRINT_BY_X_則此欄位無意義
*/
int inPRINT_Buffer_Sync_UnderLine(char* szString, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, unsigned char uszPrintPosition, int intXPosition)
{
	int		i;
	int		inMaxYLength = 0;	/* 所有中最大的高度 */
	int		inYcurrentMAX_Old;	/* 印失敗要回復原該行底線 */
	int		inFontYLength;		/* 字型高度 */
	char		szDebugMsg[100 + 1];	/* DebugMsg */
	unsigned short	usReturnValue;		/* ReturnValue */	
	
	memset(&gsrBAUL[ginBAUL_Index].srBhandle, 0x00, sizeof(BufferHandle));
	memcpy(&gsrBAUL[ginBAUL_Index].srBhandle, srBhandle, sizeof(BufferHandle));

	memset(&gsrBAUL[ginBAUL_Index].szString, 0x00, sizeof(szString));
	memcpy(&gsrBAUL[ginBAUL_Index].szString, szString, strlen(szString));

	memset(&gsrBAUL[ginBAUL_Index].srFont_Attrib, 0x00, sizeof(FONT_ATTRIB));
	memcpy(&gsrBAUL[ginBAUL_Index].srFont_Attrib, srFont_Attrib, sizeof(FONT_ATTRIB));

	if (uszPrintPosition != _PRINT_BY_X_)
	{
		/* Aligned */
		gsrBAUL[ginBAUL_Index].uszPrintPosition = uszPrintPosition;
	}
	else
	{
		/* By x位置 */
		gsrBAUL[ginBAUL_Index].uszPrintPosition = uszPrintPosition;
		gsrBAUL[ginBAUL_Index].intXPosition = intXPosition;
	}
	
	ginBAUL_Index ++;
		
	if (inNextLine == _CURRENT_LINE_)
	{
		
	}
	/* Last Entry */
	else
	{
		/* Find Max Y length */
		for (i = 0; i < ginBAUL_Index; i++)
		{
			inFontYLength = gsrBAUL[i].srFont_Attrib.FontSize % 0x0100;
			if (inMaxYLength < inFontYLength)
				inMaxYLength = inFontYLength;
		}
		
		/* 紀錄當前行底線 */
		/* 萬一失敗拿來還原用 */
		inYcurrentMAX_Old = srBhandle->inYcurrentMAX;
		/* 該行底線設為該行字型 */
		srBhandle->inYcurrentMAX = srBhandle->inYcurrent + inMaxYLength;
		
		for (i = 0; i < ginBAUL_Index; i++)
		{
			inFontYLength = gsrBAUL[i].srFont_Attrib.FontSize % 0x0100;
			
			if (gsrBAUL[i].uszPrintPosition != _PRINT_BY_X_)
			{
				usReturnValue = CTOS_PrinterBufferPutStringAligned(uszBuffer, srBhandle->inYcurrentMAX - inFontYLength, (unsigned char*)&gsrBAUL[i].szString, &gsrBAUL[i].srFont_Attrib, gsrBAUL[i].uszPrintPosition);
			}
			else
			{
				usReturnValue = CTOS_PrinterBufferPutString(uszBuffer, gsrBAUL[i].intXPosition, srBhandle->inYcurrentMAX - inFontYLength, (unsigned char*)&gsrBAUL[i].szString, &gsrBAUL[i].srFont_Attrib);
			}
			
			if (usReturnValue == d_OK)
			{
				/* 寫成功 */
			}
			else
			{
				/* 寫失敗，拿來還原用 */
				srBhandle->inYcurrentMAX = inYcurrentMAX_Old;

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Buffer_PutString failed.%s", szString);
					inDISP_LogPrintf(szDebugMsg);
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "代碼：0x%04X", usReturnValue);
					inDISP_LogPrintf(szDebugMsg);

					inDISP_LogPrintf("inPRINT_Put_In_Buffer() END!");
				}

				return (VS_ERROR);
			}
		}
		
		/* 換下一行 */
		srBhandle->inXcurrent = 0;
		srBhandle->inYcurrent = srBhandle->inYcurrentMAX;
		
		/* 清空資料暫存 */
		memset(gsrBAUL, 0x00, sizeof(BufferArrangeUnderLine) * ginBAUL_Index);
		ginBAUL_Index = 0;
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inPRINT_PutGraphic_ByBuffer
Date&Time	:2016/2/17 上午 11:10
Describe	:列印圖片
 * inCover:	_COVER_		1	用於以圖做的帳單，handle不下移
		_APPEND_		2	印完圖片，接著印下面
*/
int inPRINT_Buffer_PutGraphic(unsigned char *uszFilename, unsigned char* uszBuffer1, BufferHandle *srBhandle, int inGraphicYLength, int inCover)
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	unsigned short	usReturnValue;
	
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inPRINT_Buffer_PutGraphic() START!");
	}
	
	/* 輸入高度不合法 */
	if (inGraphicYLength <= 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Graphic height get error,FileName = %s, height = %d", uszFilename, inGraphicYLength);
			inDISP_LogPrintf(szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	/* 超過最下邊界 */
	if ((srBhandle->inYcurrent + inGraphicYLength) > srBhandle->inYbound )
	{
//		if (ginDebug == VS_TRUE)
//		{
//			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//			sprintf(szDebugMsg, "Buffer Space Not Enough To Put Graphic ");
//			inDISP_LogPrintf(szDebugMsg);
//		}
//		
//		return (VS_ERROR);
		
		/* 當放超過高度 直接OutPut並清空Buffer(保險起見) */
		do
		{
			inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, srBhandle);
		} while (inRetVal != VS_SUCCESS);
			
		memset(uszBuffer1, 0x00, srBhandle->inXbound * srBhandle->inYbound);

		srBhandle->inXcurrent = 0;
		srBhandle->inYcurrent = 0;
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Xcurrent : %d Ycurrent : %d", srBhandle->inXcurrent, srBhandle->inYcurrent);
		inDISP_LogPrintf(szDebugMsg);
	}
	
	/* 圖單獨印，所以X位置直接從0 */
	usReturnValue = CTOS_PrinterBufferBMPPic(uszBuffer1, 0, srBhandle->inYcurrent, uszFilename);

        if (usReturnValue == d_OK)
        {
		srBhandle->inXcurrent = 0;
		/* 2016/3/21 下午 12:05加入的feature，為了做出以圖片為模板的帳單 */
		if (inCover == _COVER_)
		{
			/* 當底圖handle不下移，但紀錄最低印到哪 */
			srBhandle->inYcover += srBhandle->inYcurrent + inGraphicYLength;
		}
		else
		{
			/* APPEND handle下移 */
			srBhandle->inYcurrent += inGraphicYLength;
		}
		
		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inPRINT_Put_In_Buffer() END!");
		}
		
                return (VS_SUCCESS);
        }
        else
        {
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Buffer_PutGraphic failed.%s", uszFilename);
			inDISP_LogPrintf(szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "代碼：0x%04X", usReturnValue);
			inDISP_LogPrintf(szDebugMsg);
			
			inDISP_LogPrintf("inPRINT_Buffer_PutGraphic() END!");
		}

                return (VS_ERROR);
        }
}

/*
Function	:inPRINT_Buffer_OutPut
Date&Time	:2016/3/16 下午 2:32
Describe	:將Buffer中的資料印出
*/
int inPRINT_Buffer_OutPut(unsigned char *uszBuffer, BufferHandle *srBhandle)
{
	char		szTemplate[44 + 1];
	unsigned char	uszKey;
	unsigned short	usReturnValue;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inPRINT_Buffer_OutPut() START!");
	}

	/* Q:為什麼要除以8 A:列印的高度以8為單位 Ex:12x24的字需用3行來印(24 = 8 * 3) 12x36需用5行來印(36 < 40 = 8 * 5 )*/
	if (srBhandle->inYcurrent >= srBhandle->inYcover)
		usReturnValue = CTOS_PrinterBufferOutput(uszBuffer, ((srBhandle->inYcurrent) / 8 + 1));
	else
		usReturnValue = CTOS_PrinterBufferOutput(uszBuffer, ((srBhandle->inYcover) / 8) + 1);

	if (usReturnValue == d_OK)
	{
		/* 如果開ISODebug不小心按到TMS下載會印大量電文之類的時候，狂按清除鍵解除ISODebug */
		if (ginISODebug == VS_TRUE)
		{
			if (uszKBD_Key() == _KEY_CANCEL_)
			{
				ginISODebug = VS_FALSE;
			}
		}

		if (ginDebug == VS_TRUE)
		{
			inDISP_LogPrintf("inPRINT_Buffer_OutPut() END!");
		}

		return (VS_SUCCESS);
	}
	else
	{
		if (usReturnValue == d_PRINTER_PAPER_OUT)
		{
			/* 印表機缺紙請裝紙 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ERR_PRINT_PAPER_OUT_, 0, _COORDINATE_Y_LINE_8_6_);
			inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);

			while (1)
			{
				inDISP_BEEP(1, 1000);
				uszKey = uszKBD_Key();

				if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else
				{
					continue;
				}
			}

			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf(szTemplate);
				inDISP_LogPrintf("inPRINT_Buffer_OutPut() END!");
			}

			return (VS_PRINTER_PAPER_OUT);
		}
		else if (usReturnValue == d_PRINTER_HEAD_OVERHEAT)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));	
			sprintf(szTemplate, "代碼：0x%04X", usReturnValue);	/* 錯誤代碼 */

			/* 印表機過熱 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "過熱", _LINE_8_5_);

			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf(szTemplate);
				inDISP_LogPrintf("inPRINT_Buffer_OutPut() END!");
			}

			return (VS_PRINTER_OVER_HEAT);
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));	
			sprintf(szTemplate, "代碼：0x%04X", usReturnValue);	/* 錯誤代碼 */

			/* 印表機錯誤 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, "", _LINE_8_5_);

			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf(szTemplate);
				inDISP_LogPrintf("inPRINT_Buffer_OutPut() END!");
			}

			return (VS_ERROR);
		}

	}
}

/*
Function	:inPRINT_Buffer_GetHeightFlow
Date&Time	:2016/3/16 下午 2:48
Describe	:決定要加入幾張圖，不return error是因為當有缺圖，其他圖仍能運作正常
*/
int inPRINT_Buffer_GetHeightFlow(void)
{
	int	inPreLen = 0;
	int	inPathLen = 0;
	char	szPath[100 + 1];
	char	szFileName[50 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inPRINT_Buffer_GetHeightFlow() START!");
	}
	
	/* 初始化結構 */
	memset(&gsrBMPHeight, 0x00, sizeof(BMPHeight));
	
	inPreLen = strlen("./fs_data/");
	
	/* 銀行LOGO高度 */
	memset(szPath, 0x00, sizeof(szPath));
	inPathLen = strlen(_BANK_LOGO_);
	memcpy(szPath, _BANK_LOGO_, inPathLen);
	
	memset(szFileName, 0x00, sizeof(szFileName));
	memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);
	
	inPRINT_Buffer_GetHeight((unsigned char*)szFileName, &gsrBMPHeight.inBankLogoHeight);
	
	/* 商店LOGO高度 */
	memset(szPath, 0x00, sizeof(szPath));
	inPathLen = strlen(_MERCHANT_LOGO_);
	memcpy(szPath, _MERCHANT_LOGO_, inPathLen);
	
	memset(szFileName, 0x00, sizeof(szFileName));
	memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);
	
	inPRINT_Buffer_GetHeight((unsigned char*)szFileName, &gsrBMPHeight.inMerchantLogoHeight);
	
	/* 表頭圖檔高度 */
	memset(szPath, 0x00, sizeof(szPath));
	inPathLen = strlen(_NAME_LOGO_);
	memcpy(szPath, _NAME_LOGO_, inPathLen);
	
	memset(szFileName, 0x00, sizeof(szFileName));
	memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);
	
	inPRINT_Buffer_GetHeight((unsigned char*)szFileName, &gsrBMPHeight.inTitleNameHeight);
	
	/* 企業標語高度 */
	memset(szPath, 0x00, sizeof(szPath));
	inPathLen = strlen(_SLOGAN_LOGO_);
	memcpy(szPath, _SLOGAN_LOGO_, inPathLen);
	
	memset(szFileName, 0x00, sizeof(szFileName));
	memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);
	
	inPRINT_Buffer_GetHeight((unsigned char*)szFileName, &gsrBMPHeight.inSloganHeight);
	
	/* CUP警語高度 */
	memset(szPath, 0x00, sizeof(szPath));
	inPathLen = strlen(_CUP_LEGAL_LOGO_);
	memcpy(szPath, _CUP_LEGAL_LOGO_, inPathLen);
	
	memset(szFileName, 0x00, sizeof(szFileName));
	memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);
	
	inPRINT_Buffer_GetHeight((unsigned char*)szFileName, &gsrBMPHeight.inCupLegalHeight);
	
	/* 分期警語高度 */
	memset(szPath, 0x00, sizeof(szPath));
	inPathLen = strlen(_LEGAL_LOGO_);
	memcpy(szPath, _LEGAL_LOGO_, inPathLen);
	
	memset(szFileName, 0x00, sizeof(szFileName));
	memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);
	
	inPRINT_Buffer_GetHeight((unsigned char*)szFileName, &gsrBMPHeight.inInstHeight);
	
	/* 商店提示與高度*/
	memset(szPath, 0x00, sizeof(szPath));
	inPathLen = strlen(_NOTICE_LOGO_);
	memcpy(szPath, _NOTICE_LOGO_, inPathLen);
	
	memset(szFileName, 0x00, sizeof(szFileName));
	memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);
	
	inPRINT_Buffer_GetHeight((unsigned char*)szFileName, &gsrBMPHeight.inNoticeHeight);
			
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inPRINT_Buffer_GetHeightFlow() END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inPRINT_Buffer_Get_Height
Date&Time	:2016/3/16 下午 2:15
Describe	:用來預先獲取圖檔的高度，以便利用
*/
int inPRINT_Buffer_GetHeight(unsigned char* uszFileName, int* inHeight)
{
	int		i;
	int		inCalulateHeight = 0;
	char		szAscii[8 + 1];
	char		szDebugMsg[100 + 1];
	unsigned char	uszHex[4 + 1];
	unsigned char	uszHeight[4 + 1];
	unsigned long	ulHandle;
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inPRINT_Buffer_Get_Height() START!");
	}
	
	memset(&ulHandle, 0x00, sizeof(ulHandle));
	memset(uszHeight, 0x00, sizeof(uszHeight));
	memset(uszHex, 0x00, sizeof(uszHex));
	memset(szAscii, 0x00, sizeof(szAscii));
	/* 初始為0 若因錯誤而跳出則高度為0 */
	*inHeight = 0;
	
	if (inFILE_OpenReadOnly(&ulHandle, uszFileName) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	/* 16 in hex = 22 in decimal 圖檔放高度資訊的位置 */
	if (inFILE_Seek(ulHandle, 22, _SEEK_BEGIN_)  != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inFILE_Read(&ulHandle, uszHeight, 4)  != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* height資料只有四個bytes，因為是Little-Endian，所以要先反過來 */
	for (i = 0; i < 4; i++)
	{
		memcpy(&uszHex[i], &uszHeight[3 - i], 1);
	}
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)uszHex, 4);
		sprintf(szDebugMsg, "%s", szAscii);
		inDISP_LogPrintf(szDebugMsg);
	}
	
	/* 16進制轉10進制，以byte為單位，從高位乘上256加到低位 */
	for (i = 0; i < 4 ; i++)
	{
		inCalulateHeight *= 256;
		inCalulateHeight += uszHex[i] - 0;
	}
	
	/* 加到變數上 */
	*inHeight = inCalulateHeight;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%d", inCalulateHeight);
		inDISP_LogPrintf(szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDISP_LogPrintf("inPRINT_Buffer_Get_Height() END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inPRINT_Buffer_Barcode
Date&Time	:2017/2/14 下午 4:56
Describe	:列印Barcode Buffer
*/
int inPRINT_Buffer_Barcode(char *szBarcode, unsigned char *uszBuffer, BufferHandle *srBhandle, unsigned short usX, unsigned char uszShowChar)
{
	char		szBarCodeFormat[100 + 1];
	char		szTemplate[44 + 1];
	unsigned char	uszKey;
        unsigned short	usRetVal;

	/* API要求前後要加"*" (星號) */
	memset(szBarCodeFormat, 0x00, sizeof(szBarCodeFormat));
	sprintf(szBarCodeFormat, "%s", szBarcode);
	
	usRetVal = CTOS_PrinterBufferCode39Barcode(usX, srBhandle->inYcurrent, (unsigned char*)szBarcode, strlen(szBarcode), 2, 3, uszShowChar); /* TRUE 顯示裡面的字 FLASE 不顯示 */

        if (usRetVal == d_OK)
        {
		srBhandle->inYcurrent += 80;
		/* 顯示字元要多留空間 */
		if (uszShowChar == TRUE)
		{
			srBhandle->inYcurrent += 8;
		}
		
                return (VS_SUCCESS);
        }
        else
	{
                if (usRetVal == d_PRINTER_PAPER_OUT)
                {
			/* 印表機缺紙請裝紙 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ERR_PRINT_PAPER_OUT_, 0, _COORDINATE_Y_LINE_8_6_);
			inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
			
			while (1)
			{
				inDISP_BEEP(1, 1000);
				uszKey = uszKBD_Key();

				if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else
				{
					continue;
				}
			}
			
			return (VS_PRINTER_PAPER_OUT);
                }
		else if (usRetVal == d_PRINTER_HEAD_OVERHEAT)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));	
                        sprintf(szTemplate, "印表機過熱");	/* 錯誤代碼 */
			
			/* 印表機過熱 */
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
			
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf(szTemplate);
				inDISP_LogPrintf("inPRINT_Buffer_OutPut() END!");
			}
			
			return (VS_PRINTER_OVER_HEAT);
		}
                else
                {
			/* 印表機錯誤 */
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
                }

                return (VS_ERROR);

        }
}

/*
Function	:inPRINT_Buffer_QRcode
Date&Time	:2017/2/14 下午 4:56
Describe	:列印QRcode Buffer
*/
int inPRINT_Buffer_QRcode(char *szQRcode, unsigned char *uszBuffer, BufferHandle *srBhandle, unsigned short usX)
{
//#ifndef	_LOAD_KEY_AP_

	int			inBaseSize = 0;
	char			szQRCodeFormat[200 + 1];
	char			szTemplate[44 + 1];
	unsigned char		uszKey;
        unsigned short		usRetVal;
	CTOS_QRCODE_INFO	srQR_INFO;

	srQR_INFO.InfoVersion = QR_INFO_VERSION;
	srQR_INFO.Size = 3;				/* 縮放 5預設值 10:兩倍 1:五分之一 */
	srQR_INFO.Version = QR_VERSION45X45;		/* QR Code的大小 21 x 21 到 177 x 177 財政部查電子發票 至少要V6以上 */
	srQR_INFO.Level = QR_LEVEL_M;			/* 錯誤修正(資料回復能力) Level L M Q H 商業用途: L or M 工業環境: Q or H 設定愈高影響可儲存的資料量*/
	
	/* 根據ctos_qrcode.h每加一，值多四 */
	inBaseSize = 17 + (srQR_INFO.Version * 4);
	
	/* API要求前後要加"*" (星號) */
	memset(szQRCodeFormat, 0x00, sizeof(szQRCodeFormat));
	sprintf(szQRCodeFormat, "%s", szQRcode);
	
	usRetVal = CTOS_QRCodePrinterBuffer(&srQR_INFO, szQRcode, usX, srBhandle->inYcurrent);

	if (usRetVal != d_OK)
	{
		if (usRetVal == d_PRINTER_PAPER_OUT)
		{
			/* 印表機缺紙請裝紙 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ERR_PRINT_PAPER_OUT_, 0, _COORDINATE_Y_LINE_8_6_);
			inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
			
			while (1)
			{
				inDISP_BEEP(1, 1000);
				uszKey = uszKBD_Key();

				if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else
				{
					continue;
				}
			}
			
			return (VS_PRINTER_PAPER_OUT);
                }
		else if (usRetVal == d_PRINTER_HEAD_OVERHEAT)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));	
                        sprintf(szTemplate, "印表機過熱");	/* 錯誤代碼 */
			
			/* 印表機過熱 */
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
			
			if (ginDebug == VS_TRUE)
			{
				inDISP_LogPrintf(szTemplate);
				inDISP_LogPrintf("inPRINT_Buffer_OutPut() END!");
			}
			
			return (VS_PRINTER_OVER_HEAT);
		}
                else
                {
			/* 印表機錯誤 */
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_Msg_BMP(_ERR_PRINT_, _COORDINATE_Y_LINE_8_6_, _ENTER_KEY_MSG_, _EDC_TIMEOUT_, szTemplate, _LINE_8_5_);
                }

                return (VS_ERROR);

        }
	else
	{
		srBhandle->inYcurrent += inBaseSize * srQR_INFO.Size;		
	}

//#endif

	return (VS_SUCCESS);
}

/*
Function        :inPRINT_ChineseFont_Format
Date&Time       :2017/1/10 上午 10:35
Describe        :達到一定長度後自動切斷,debug訊息用
*/
int inPRINT_ChineseFont_Format(char* szPrintBuffer, char* szPadData, int inOneLineLen, int inFontSize)
{
	int	inPrintLineBufferSize = 36 + 1;
	int	inPrintLineCnt = 0;
	char	szPrtBuf[50 + 1], szPrintLineData[inPrintLineBufferSize];
		
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

		inPRINT_ChineseFont(szPrtBuf, _PRT_ISO_);
		inPrintLineCnt ++;
	};
	
	return (VS_SUCCESS);
}

///*
//Function        :inPRINT_ChineseFont_Format
//Date&Time       :2017/1/10 上午 10:35
//Describe        :達到一定長度後自動切斷,debug訊息用
//*/
//int inPRINT_Buffer_Format_PutIn(char* szPrintBuffer, char* szPadData, int inOneLineLen, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
//{
//	int	inPrintLineBufferSize = 36 + 1;
//	int	inPrintLineCnt = 0;
//	char	szPrtBuf[50 + 1], szPrintLineData[inPrintLineBufferSize];
//		
//	inPrintLineCnt = 0;
//	while ((inPrintLineCnt * inOneLineLen) < strlen(szPrintBuffer))
//	{
//		memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
//		memset(szPrtBuf, 0x00, sizeof(szPrtBuf));
//		if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szPrintBuffer))
//		{
//			strcat(szPrintLineData, &szPrintBuffer[inPrintLineCnt * inOneLineLen]);
//		}
//		else
//		{
//			memcpy(szPrintLineData, &szPrintBuffer[inPrintLineCnt * inOneLineLen], inOneLineLen);
//		}
//		
//		sprintf(szPrtBuf, "%s%s", szPadData, szPrintLineData);
//
//		inPrintLineCnt ++;
//	};
//	
//	return (VS_SUCCESS);
//}

int inPRINT_Receipt_Test1(void)
{
    	unsigned char uszKey;
        char szPrintBuf[48 + 1], szTemplate[48 + 1];

        inPRINT_Initial();
        inPRINT_TTF_SetFont(_PRT_CHINESE_1_, _FONT_REGULAR_);

        /* 列印商店LOGO */
        inPRINT_PutGraphic((unsigned char*)"./fs_data/NcccLogo.bmp");
        inPRINT_PutGraphic((unsigned char*)"./fs_data/BmpLogo.bmp");
        /* 列印商店代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        inFunc_PAD_ASCII(szTemplate, "6601000081", ' ', 30, _PAD_RIGHT_FILL_LEFT_);
        sprintf(szPrintBuf, "商店代號　%s", szTemplate);
        inDISP_LogPrintf(szPrintBuf);

        //inDISP_LogPrintf("商店代號　　　　　 　6601000081", _PRT_DOUBLE_HEIGHT_WIDTH_);
	/* 列印端末機代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        inFunc_PAD_ASCII(szTemplate, "13995512", ' ', 32, _PAD_RIGHT_FILL_LEFT_);
        sprintf(szPrintBuf, "端末機代號%s", szTemplate);
        inDISP_LogPrintf(szPrintBuf);
        //inDISP_LogPrintf("端末機代號　　　　　　 13995512", _PRT_DOUBLE_HEIGHT_WIDTH_);
        inDISP_LogPrintf("================================================");
        /* 城市別(City) */
        inDISP_LogPrintf("城市別(City)");
        inDISP_LogPrintf("TAIPEI");
	/* 卡別 檢查碼 */
        inDISP_LogPrintf("卡別(Card Type )      　　 檢查碼(Check No.)");
        inDISP_LogPrintf("JCB　　　　　　　67635");
        /* 卡號 */
        inDISP_LogPrintf("卡號(Card No.)");
        inDISP_LogPrintf("3560500100001218");
        /* 主機別*/
        inDISP_LogPrintf("主機別/交易類別(Host/Trans. Type)");
        inDISP_LogPrintf("NCCC  00  一般交易  SALE");
        /* 批次號碼 授權碼*/
        inDISP_LogPrintf("批次號碼(Batch No.)     　 授權碼(Auth Code)");
        inDISP_LogPrintf("001　　　　　　　 777777");
        /* 日期時間 */
        inDISP_LogPrintf("日期/時間(Date/Time)");
        inDISP_LogPrintf("2015/05/13   12:35");
        /* 序號 調閱編號 */
        inDISP_LogPrintf("序號(Ref. No.)　　　　　調閱編號(Inv. No.)");
        inDISP_LogPrintf("99551201001　　000001");
        /* 櫃號 */
        inDISP_LogPrintf("櫃號(Store ID)");
        inDISP_LogPrintf("12345678900");
        /* 金額 */
        sprintf(szPrintBuf, "%d", 250);
        sprintf(szTemplate, "金額(Amount): NT$ 2500");
        inDISP_LogPrintf(szTemplate);
        /* 小費 */
        inDISP_LogPrintf("小費(Tips)  :_________________________");

        /* 總計 */
        inDISP_LogPrintf("總計(Total) :_________________________");
        /* 簽名欄 */
        inPRINT_SpaceLine(2);  //a space 2 line
        inDISP_LogPrintf("X:________________________________");
        inDISP_LogPrintf("                                                    持卡人簽名");
        inDISP_LogPrintf("                                                      商店存根");
        inDISP_LogPrintf("--------------------------------------------------------------------------------------------------------------");
        inDISP_LogPrintf("                                 I AGREE TO PAY TOTAL AMOUNT");
        inDISP_LogPrintf("                       ACCORDING TO CARD ISSUER AGREEMENT");
        inPRINT_SpaceLine(6);

        do
        {
                CTOS_KBDGet(&uszKey);
        } while(uszKey == d_KBD_ENTER);

        inPRINT_SpaceLine(2);
        /* 第二聯 */
        /* 列印商店LOGO */
        inPRINT_PutGraphic((unsigned char*)"./fs_data/NcccLogo.bmp");
        inPRINT_PutGraphic((unsigned char*)"./fs_data/BmpLogo.bmp");
        /* 列印商店代號 */
        inDISP_LogPrintf("商店代號　　　　　 　6601000081");
	/* 列印端末機代號 */
        inDISP_LogPrintf("端末機代號　　　　　　 13995512");
        inDISP_LogPrintf("================================================");
        /* 城市別(City) */
        inDISP_LogPrintf("城市別(City)");
        inDISP_LogPrintf("TAIPEI");
	/* 卡別 檢查碼 */
        inDISP_LogPrintf("卡別(Card Type )      　　 檢查碼(Check No.)");
        inDISP_LogPrintf("JCB　　　　　　　67635");
        /* 卡號 */
        inDISP_LogPrintf("卡號(Card No.)");
        inDISP_LogPrintf("3560500100001218");
        /* 主機別*/
        inDISP_LogPrintf("主機別/交易類別(Host/Trans. Type)");
        inDISP_LogPrintf("NCCC  00  一般交易  SALE");
        /* 批次號碼 授權碼*/
        inDISP_LogPrintf("批次號碼(Batch No.)     　 授權碼(Auth Code)");
        inDISP_LogPrintf("001　　　　　　　 777777");
        /* 日期時間 */
        inDISP_LogPrintf("日期/時間(Date/Time)");
        inDISP_LogPrintf("2015/05/13   12:35");
        /* 序號 調閱編號 */
        inDISP_LogPrintf("序號(Ref. No.)　　　　　調閱編號(Inv. No.)");
        inDISP_LogPrintf("99551201001　　000001");
        /* 櫃號 */
        inDISP_LogPrintf("櫃號(Store ID)");
        inDISP_LogPrintf("12345678900");
        /* 金額 */
        sprintf(szPrintBuf, "%d", 250);
        sprintf(szTemplate, "金額(Amount): NT$ 2500");
        inDISP_LogPrintf(szTemplate);
        /* 小費 */
        inDISP_LogPrintf("小費(Tips)  :_________________________");

        /* 總計 */
        inDISP_LogPrintf("總計(Total) :_________________________");
        /* 簽名欄 */
        inPRINT_SpaceLine(2);  //a space 2 line
        inDISP_LogPrintf("X:________________________________");
        inDISP_LogPrintf("                                                    持卡人存根");
        inDISP_LogPrintf("--------------------------------------------------------------------------------------------------------------");
        inDISP_LogPrintf("                                 I AGREE TO PAY TOTAL AMOUNT");
        inDISP_LogPrintf("                       ACCORDING TO CARD ISSUER AGREEMENT");
        inPRINT_SpaceLine(8);

        return (VS_SUCCESS);

}

int inPRINT_Receipt_Test2(void)
{
    	unsigned char uszKey;

        inPRINT_Initial();
        inPRINT_TTF_SetFont(_PRT_CHINESE_1_, _FONT_REGULAR_);

        inPRINT_PutGraphic((unsigned char*)"./fs_data/NcccLogo.bmp");
        inPRINT_PutGraphic((unsigned char*)"./fs_data/BmpLogo.bmp");
        inDISP_LogPrintf("信用卡");
        inDISP_LogPrintf("卡別：AMEX");
        inDISP_LogPrintf("商店代號：1900586");
        inDISP_LogPrintf("端末機代號：13999197");
        inDISP_LogPrintf("卡號：376348129193008(s)");
        inDISP_LogPrintf("日期　　：2015/05/19");
        inDISP_LogPrintf("時間　　：14:07:58");
        inDISP_LogPrintf("調閱編號：000023");
        inDISP_LogPrintf("批次號碼：000010");
        inDISP_LogPrintf("交易類別：SALE");
        inDISP_LogPrintf("授權碼　：60");
        inDISP_LogPrintf("序號　　：99919710047");
        inDISP_LogPrintf("");
        inDISP_LogPrintf("金額AMOUNT：NT$      250");
        inDISP_LogPrintf("小費Tips :__________________");
        inPRINT_SpaceLine(2);
        inDISP_LogPrintf("簽名欄:_____________________");
        inDISP_LogPrintf("*** 商店收據 Merchant Copy ***");
        inDISP_LogPrintf("I AGREE TO PAY TOTAL AMOUNT");
        inDISP_LogPrintf("ACCORDING TO CARD ISSUER AGREEMENT");
        inPRINT_SpaceLine(6);

        do
        {
                CTOS_KBDGet(&uszKey);
        }while(uszKey == d_KBD_ENTER);

        inPRINT_SpaceLine(2);

        inPRINT_PutGraphic((unsigned char*)"./fs_data/NcccLogo.bmp");
        inPRINT_PutGraphic((unsigned char*)"./fs_data/BmpLogo.bmp");
        inDISP_LogPrintf("信用卡");
        inDISP_LogPrintf("卡別：AMEX");
        inDISP_LogPrintf("商店代號：1900586");
        inDISP_LogPrintf("端末機代號：13999197");
        inDISP_LogPrintf("卡號：376348129193008(s)");
        inDISP_LogPrintf("日期　　：2015/05/19");
        inDISP_LogPrintf("時間　　：14:07:58");
        inDISP_LogPrintf("調閱編號：000023");
        inDISP_LogPrintf("批次號碼：000010");
        inDISP_LogPrintf("交易類別：SALE");
        inDISP_LogPrintf("授權碼　：60");
        inDISP_LogPrintf("序號　　：99919710047");
        inDISP_LogPrintf("");
        inDISP_LogPrintf("金額AMOUNT：NT$      250");
        inDISP_LogPrintf("小費Tips :__________________");
        inPRINT_SpaceLine(2);
        inDISP_LogPrintf("簽名欄:_____________________");
        inDISP_LogPrintf("*** 持卡人收據 Customer Copy ***");
        inDISP_LogPrintf("I AGREE TO PAY TOTAL AMOUNT");
        inDISP_LogPrintf("ACCORDING TO CARD ISSUER AGREEMENT");
        inPRINT_SpaceLine(8);

        return (VS_SUCCESS);
}

int inPRINT_EI_Test1(void)
{
        char szQR1[256 + 1];
        char szQR2[256 + 1];

        inPRINT_Initial();
        inPRINT_TTF_SetFont(_PRT_CHINESE_1_, _FONT_REGULAR_);

        /* 列印商店LOGO */
        inPRINT_PutGraphic((unsigned char*)"./fs_data/BmpLogo.bmp");

        inDISP_LogPrintf("  電子發票證明聯");
        inDISP_LogPrintf("   104年05-06月");
        inDISP_LogPrintf("   VR-73046176");
        inDISP_LogPrintf("2015-06-06  19:39:06");
        inDISP_LogPrintf("隨機碼：7418　總計：35");
        inDISP_LogPrintf("賣方16443375");
        inPRINT_SpaceLine(1);
        inPRINT_Barcode("11F951FDB7C0441CB85A");
        strcpy(szQR1,"VR730461761040606741800000000000000230000000016443375sYYPdXCdl8HJ8xKJXvJnQg==:**************:1:1:1:健達出奇蛋");
        strcpy(szQR2,"**");
        inPRINT_SpaceLine(1);
        inPRINT_TwoQR(szQR1, szQR2);
        inDISP_LogPrintf("統和    128911  序688533  機2");
        inDISP_LogPrintf("退貨憑電子發票證明聯正本辦理");
        inPRINT_SpaceLine(8);

        return (VS_SUCCESS);
}

/*
Function	:inPRINT_Buffer_Barcode_Test
Date&Time	:2017/2/14 下午 4:56
Describe	:列印Barcode Buffer
*/
int inPRINT_Buffer_Barcode_Test()
{
	int			inYcurrent;
	char			szPrintBuf[84 + 1];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	
	memset(uszBuffer, 0x00, PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_);
	
	
	inYcurrent = 0;
	
	CTOS_PrinterBufferInit(uszBuffer, 8 * _BUFFER_MAX_LINE_);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s", "11B83F1B852B4E839377");
	
	CTOS_PrinterBufferCode39Barcode(0, inYcurrent, (unsigned char*)szPrintBuf, strlen(szPrintBuf), 2, 3, TRUE); /* TRUE 顯示裡面的字 FLASE 不顯示 */
	inYcurrent += 88;
	
	CTOS_PrinterBufferOutput(uszBuffer, (inYcurrent / 8) + 1);
	
	return (VS_SUCCESS);
}

/*
Function	:inPRINT_Buffer_Barcode_Test2
Date&Time	:2017/2/17 上午 11:32
Describe	:列印QRcode Buffer Test
*/
int inPRINT_Buffer_Barcode_Test2()
{
	int			inYcurrent;
	char			szPrintBuf[84 + 1];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	FONT_ATTRIB		srFont_Attrib;
	BufferHandle		srBhandle;
	
	memset(uszBuffer, 0x00, PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_);
	memset(&srBhandle, 0x00, sizeof(BufferHandle));
	
	
	inYcurrent = 0;
	
	inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s", "11B83F1B852B4E839377");
//	inPRINT_Buffer_PutIn("一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, &srBhandle, 0, TRUE);
//	inPRINT_Buffer_PutIn("一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	
	return (VS_SUCCESS);
}

/*
Function	:inPRINT_Buffer_QRcode_Test
Date&Time	:2017/2/17 上午 9:08
Describe	:列印QRcode Buffer Test
*/
int inPRINT_Buffer_QRcode_Test()
{
	int			inYcurrent;
	char			szPrintBuf[84 + 1];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	FONT_ATTRIB		srFont_Attrib;
	BufferHandle		srBhandle;
	
	memset(uszBuffer, 0x00, PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_);
	memset(&srBhandle, 0x00, sizeof(BufferHandle));
	
	
	inYcurrent = 0;
	
	inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s", "www.google.com.tw");
	inPRINT_Buffer_PutIn("一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_QRcode(szPrintBuf, uszBuffer, &srBhandle, 0);
	inPRINT_Buffer_PutIn("一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	
	return (VS_SUCCESS);
}


/*
Function	:  inPRINT_Debug_Buffer
Date&Time :  2021/12/14 上午 10:12
Describe	: 列印DEBUG資料用，用完一定要刪掉。
*/
int inPRINT_Debug_Buffer(char *szPrintTitle ,char *szStr, ...)
{
        char szDebugMsg[500 + 1] = {0};
        va_list		list;	/* 加入不定參數作法 2018/5/29 上午 11:06 */
        
        inPRINT_ChineseFont(szPrintTitle,_PRT_NORMAL_);
        
        va_start(list, szStr);
        vsprintf(szDebugMsg, szStr, list);
        va_end(list);
        printf(szDebugMsg);
        
        inPRINT_ChineseFont(szDebugMsg,_PRT_NORMAL_);
        
        return (VS_SUCCESS);
}

void inPRINT_LogPrintfArea(char fDebugFlag,char *chMsg,int inMsgLen,BYTE *chData,int inDataLen)
{
        BYTE *bBuf;
        int i;

//      if (ginDebug == VS_TRUE)
        {
                bBuf = (BYTE *)calloc(1, inDataLen * 4 + inMsgLen + 10);
                memset(bBuf, 0x00, inDataLen * 3 + inMsgLen + 10);
                memcpy(bBuf,chMsg,inMsgLen);
                sprintf((char *)&bBuf[strlen((char *)bBuf)]," (%d):",inDataLen);
                for(i=0;i<inDataLen;i++)
                {
                        if(fDebugFlag ==FALSE)
                        {
                                if(chData[i] >= 0x20 && chData[i] <= 0x7F)
                                        bBuf[strlen((char *)bBuf)] = chData[i];
                                else
                                        sprintf((char *)&bBuf[strlen((char *)bBuf)],"[%02X]",chData[i]);
                        }
                        else
                                sprintf((char *)&bBuf[strlen((char *)bBuf)],"%02X",chData[i]);
                }
		inPRINT_ChineseFont((char*)bBuf, _PRT_NORMAL_);
                free(bBuf);
        }
}


