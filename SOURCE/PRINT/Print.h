/* 
 * File:   Print.h
 * Author: SuLH
 *
 * Created on 2015年6月8日, 上午 10:12
 */

#define d_LINE_DOT			12                  /* A line is 12 dots */

/* 定義中英文列印的字型 */
#if defined	_LOAD_KEY_AP_
	#define _PRT_CHINESE_FONE_1_		"ca_default.ttf"	/* 虹堡預設字體 */
#else
	#define _PRT_CHINESE_FONE_1_		"hkzht_p.ttf"		/* 華康中黑體(等寬) */
//	#define _PRT_CHINESE_FONE_1_		"GENJYUUGITHIC.ttf"
#endif

#define _PRT_CHINESE_FONE_2_		"pan.ttf"	/* 對卡號做特別加粗處理 */

/* 定義選擇中英文 */
#define _PRT_CHINESE_1_			1
#define _PRT_CHINESE_2_			2
#define _PRT_ENGLISH_			3

/* 定義字型Style */
#define _FONT_REGULAR_                  0
#define _FONT_ITALIC_                   1
#define _FONT_BOLD_                     2
#define _FONT_BOLD_ITALIC               3

/* 定義中英文列印字體大小 */
#define _PRT_NORMAL_				d_FONT_8x16
#define _PRT_NORMAL2_				0x080F			/* d_FONT_8x15(_PRT_DOUBLE_HEIGHT_WIDTH_的一半) */
#define _PRT_HEIGHT_SMALL_			0x0818			/* 08x30縮小簽單用 */
#define _PRT_WIDTH_SMALL_			0x1018			/* 16x30縮小簽單用 */
#define _PRT_HEIGHT_				d_FONT_12x24
#define _PRT_WIDTH_					d_FONT_16x16
#define _PRT_DOUBLE_HEIGHT_WIDTH_	d_FONT_16x30
#define _PRT_HEIGHT_HEIGHT_			0x0C30
#define _PRT_DOUBLE_WIDTH_WIDTH_	0x121E
#define _PRT_EI_HEIGHT_WIDTH_			0x203C			/* 32 x 60 */
#define _PRT_ISO_					0x0A18

/* Print buffer 使用 START */
#define _CURRENT_LINE_		0
#define _LAST_ENTRY_		1
/* inCover */
#define _COVER_			1	/* 用於以圖做的帳單，handle不下移 */
#define _APPEND_			2	/* 印完圖片，接著印下面*/

#define _MAX_Y_LENGTH_	32
#define _BUFFER_MAX_LINE_	800	/* 自行定義足夠大的Buffer */

/* Aligned */
#define _PRINT_LEFT_		d_PRINTER_ALIGNLEFT
#define _PRINT_CENTER_	d_PRINTER_ALIGNCENTER
#define _PRINT_RIGHT_		d_PRINTER_ALIGNRIGHT
#define _PRINT_BY_X_		127

/* Print 定義 特殊 X座標 */
#define _PRINT_DEFINE_X_01_		192			/* 定義Print特定座標點 */
#define _PRINT_DEFINE_X_02_		216			/* 定義Print特定座標點 */
#define _PRINT_DEFINE_X_03_		120			/* QRCODE_45X45置中使用 */
#define _PRINT_DEFINE_X_04_		235			/* 修改字體後放大明細報表用 */
/* Print buffer 使用 END */

#define FONT_ATTRIB			CTOS_FONT_ATTRIB
#define	QRCODE_INFO			CTOS_QRCODE_INFO
///* 定義圖檔名字 */(暫不使用)
//#define _NAME_NCCC_LOGO_		"NCCCLOGO.bmp"
//#define _NAME_MERCHANT_LOGO_		"BMPLOGO.bmp"
//#define _NAME_NAME_LOGO_		"BMPNAME.bmp"
//#define _NAME_SLOGAN_LOGO_		"BMPSLOGAN.bmp"
//#define _NAME_CUP_LEGAL_LOGO_		"CUPLEGAL.bmp"
//#define _NAME_LEGAL_LOGO_		"BMPLEGAL.bmp"
//#define _NAME_NOTICE_LOGO_		"BMPNOTICE.bmp"
        
/* 用來控制buffer位置的移動，bound是用來控制不要超出紙張或超出定義的Buffer，current表示handle現在於Buffer的什麼位置 */


typedef struct					
{
	int	inXbound;	/* 用來紀錄buffer的寬度，inXcurrent不應超過它 */
	int	inYbound;	/* 用來紀錄buffer的高度，inXcurrent不應超過它 */
	int	inXcurrent;	/* handle目前的X位置 */
	int	inYcurrent;	/* handle目前的Y位置 */
	int	inYcurrentMAX;	/* 紀錄當前這一行，最下面的位置(當同一行有印大字小字，用來表示底線在哪) */
	int	inYcover;	//如果有底圖，則要與inYcurrent比較那一個比較下面
}BufferHandle;

//typedef struct					
//{
//	int			inWay;			/* 方式：1:By 座標 2:靠左 or 靠右 */
//	int			intXPosition;		/* X軸位置 */
//	unsigned char		uszPrintPosition;	/* 靠左或靠右 */
//}PRINT_POSITION;
//
//typedef struct					
//{
//	BufferHandle		srBhandle;
//	FONT_ATTRIB		srFont_Attrib;		/* 字型資訊 */
//	PRINT_POSITION		srPrintPosition;
//}PRINT_OBJECT;

/* 用來控制PutIn在同一底線上，所以先把該行要Putin的字先存起來 */
typedef struct					
{	BufferHandle	srBhandle;
	char			szString[300];		/* 暫放的字句 (因優惠兌換資訊，所以上調到300) */
	FONT_ATTRIB	srFont_Attrib;		/* 字型資訊 */
	unsigned char	uszPrintPosition;	/* 靠左或靠右 */
	int			intXPosition;		/* X軸位置 */
}BufferArrangeUnderLine;

/* 開機時會把圖檔的高度讀出，並放在裡面 */
typedef struct
{
	int	inBankLogoHeight;	/* 銀行LOGO高度 */
	int	inMerchantLogoHeight;	/* 商店LOGO高度 */
	int	inTitleNameHeight;	/* 表頭圖檔高度 */
	int	inSloganHeight;		/* 企業標語高度 */
	int	inCupLegalHeight;	/* CUP警語高度 */
	int	inInstHeight;		/* 分期警語高度 */
	int	inNoticeHeight;		/* 商店提示與高度 */
}BMPHeight;

int inPRINT_Initial(void);
int inPRINT_TTF_SetFont(int inLanguage, int inFontStyle);
int inPRINT_SpaceLine(int inLine);
int inPRINT_PutGraphic(unsigned char *uszFilename);
int inPRINT_ChineseFont(char *szStr, int inFontSize);
//int inPRINT_EnglishFont(char *szStr, int FontSize);		不建議再使用，為了輕量化，英文也用中文字型印
int inPRINT_TwoQR(char *szQR_1, char *szQR_2);
int inPRINT_Barcode(char *szBarcode);
int inPRINT_Buffer_Initial(unsigned char *uszBuffer, int inYLength, FONT_ATTRIB *szFont_Attrib, BufferHandle *bhandle);
int inPRINT_Buffer_PutIn(char* szString, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, unsigned char uszPrintPosition);
int inPRINT_Buffer_PutIn_Specific_X_Position(char* szString, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, int intXPosition);
int inPRINT_Buffer_PutIn_Format_Auto_Change_Line(char* szString, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, unsigned char uszPrintPosition);
int inPRINT_Buffer_Sync_UnderLine(char* szString, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, unsigned char uszPrintPosition, int intXPosition);
int inPRINT_Buffer_PutGraphic(unsigned char *uszFilename, unsigned char* uszBuffer1, BufferHandle *bhandle, int inGraphicYLength, int inCover);
int inPRINT_Buffer_OutPut(unsigned char *uszBuffer, BufferHandle *srBhadle);
int inPRINT_Buffer_GetHeightFlow(void);
int inPRINT_Buffer_GetHeight(unsigned char* szFileName, int* inHeight);
int inPRINT_Buffer_Barcode(char *szBarcode, unsigned char *uszBuffer, BufferHandle *srBhandle, unsigned short usX, unsigned char uszShowChar);
int inPRINT_Buffer_QRcode(char *szQRcode, unsigned char *uszBuffer, BufferHandle *srBhandle, unsigned short usX);

/* Debug Printf 用 */
int inPRINT_ChineseFont_Format(char* szPrintBuffer, char* szPadData, int inOneLineLen, int inFontSize);

int inPRINT_Receipt_Test1(void);
int inPRINT_Receipt_Test2(void);
int inPRINT_EI_Test1(void);
int inPRINT_Buffer_Barcode_Test(void);
int inPRINT_Buffer_Barcode_Test2(void);
int inPRINT_Buffer_QRcode_Test(void);
int inPRINT_Debug_Buffer(char *szPrintTitle ,char *szStr, ...);


