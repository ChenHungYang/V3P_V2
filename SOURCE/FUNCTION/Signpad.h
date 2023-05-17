

//#pragma pack(1)/*必須在結構體定義之前使用,這是為了讓結構體中各成員按1字節對齊*/
/*定義一個存儲頭文件數據結構體*/
typedef struct tagBITMAPFILEHEADER
{
unsigned short bfType;//保存圖片類型。'BM'
unsigned long bfSize;//位圖文件的大小，以字節為單位（3-6字節，低位在前）
unsigned short bfReserved1;//位圖文件保留字，必須為0(7-8字節）
unsigned short bfReserved2;//位圖文件保留字，必須為0(9-10字節）
unsigned long bfOffBits;//RGB數據偏移地址,位圖數據的起始位置，以相對於位圖（11-14字節，低位在前）
}BITMAPFILEHEADER;

/* 定義一個存儲位圖信息的結構體 */
typedef struct tagBITMAPINFOHEADER
{
unsigned long biSize;//本結構所佔用字節數（15-18字節）
unsigned long biWidth;//位圖的寬度，以像素為單位（19-22字節）
unsigned long biHeight;//位圖的高度，以像素為單位（23-26字節）
unsigned short biPlanes;//目標設備的級別，必須為1(27-28字節）
unsigned short biBitCount;//每個像素所需的位數，必須是1（雙色）（29-30字節）,4(16色），8(256色）16(高彩色)或24（真彩色）之一
unsigned long biCompression;//位圖壓縮類型，必須是0（不壓縮），（31-34字節）
//1(BI_RLE8壓縮類型）或2(BI_RLE4壓縮類型）之一

unsigned long biSizeImage;//位圖的大小(其中包含了為了補齊行數是4的倍數而添加的空字節)，以字節為單位（35-38字節）
unsigned long biXPelsPerMeter;//位圖水平分辨率，每米像素數（39-42字節）
unsigned long biYPelsPerMeter;//位圖垂直分辨率，每米像素數（43-46字節)
unsigned long biClrUsed;//位圖實際使用的顏色表中的顏色數（47-50字節）
unsigned long biClrImportant;//位圖顯示過程中重要的顏色數（51-54字節）
}BITMAPINFOHEADER;

#define	SignBMPFile		"Signature.bmp"
#define	AmountBMPFile		"Amount.bmp"
#define	TimeBMPFile		"Time.bmp"

#define	TempSignBmpFile	"TempSign.bmp"
#define	ZoomBmpFile		"Zoom.bmp"

#define _SIGNATURE_BACKGROUND_LEFT_	"./fs_data/NEWUI/SIGNPAD_LEFT.bmp"
#define _SIGNATURE_BACKGROUND_RIGHT_	"./fs_data/NEWUI/SIGNPAD_RIGHT.bmp"

#define _SIGNATURE_CHECK_SIGN_LEFT_    "./fs_data/NEWUI/CHECK_EC_SIGN_LEFT.bmp"	/* 確認電子簽名左側畫面 20190701 [SAM] */
#define _SIGNATURE_CHECK_SIGN_RIGHT_ "./fs_data/NEWUI/CHECK_EC_SIGN_RIGHT.bmp"	/* 確認電子簽名右側畫面 20190701 [SAM] */



#define _SIGNPAD_LEFT_			1
#define _SIGNPAD_RIGHT_			2

#define _SIGNPAD_HAS_BEEN_SELECT_	 99
#define _SIGNPAD_STATUS_INIT_		 -1

/* 簽名狀態 此編號按照ESC規格 要改請謹慎 */
#define	_SIGN_NONE_			0		/* 應簽但還沒簽((初始狀態) */
#define	_SIGN_SIGNED_			1		/* 應簽而有簽 */
#define	_SIGN_NO_NEED_SIGN_	2		/* 免簽而沒簽 */
#define	_SIGN_BYPASS_			3		/* 應簽而沒簽 */

/* 簽名板線框寬度 */
#define _LINE_WIDTH_			4

#define _SIGNEDPAD_WIDTH_		180
#define _SIGNEDPAD_HIGHT_		320


/* 設定畫線的啟始點 */
#define _RIGHT_SIGNEDPAD_WRITE_LINE_X_		70
#define _LEFT_SIGNEDPAD_WRITE_LINE_X_		250 


#define _SIGNEDPAD_OUT_WIDTH_		_SIGNEDPAD_WIDTH_	+ (_LINE_WIDTH_ * 2)
#define _SIGNEDPAD_OUT_HIGHT_		_SIGNEDPAD_HIGHT_	+ (_LINE_WIDTH_ * 2)

/* 如果要置中，則x起始點為(_LCD_XSIZE_ - _SIGNEDPAD_OUT_WIDTH_) / 2 */
#define	_SIGNEDPAD_OUT_LEFT_X1_		(_LCD_XSIZE_ - _SIGNEDPAD_OUT_WIDTH_) / 2
#define	_SIGNEDPAD_OUT_LEFT_Y1_		(_LCD_YSIZE_ - _SIGNEDPAD_OUT_HIGHT_) / 2

#define	_SIGNEDPAD_OUT_RIGHT_X1_	(_LCD_XSIZE_ - _SIGNEDPAD_OUT_WIDTH_) / 2
#define	_SIGNEDPAD_OUT_RIGHT_Y1_	(_LCD_YSIZE_ - _SIGNEDPAD_OUT_HIGHT_) / 2

#define	_SIGNEDPAD_LEFT_X1_		_SIGNEDPAD_OUT_LEFT_X1_	+ _LINE_WIDTH_
#define	_SIGNEDPAD_LEFT_Y1_		_SIGNEDPAD_OUT_LEFT_Y1_	+ _LINE_WIDTH_
#define	_SIGNEDPAD_LEFT_X2_		_SIGNEDPAD_LEFT_X1_	+ _SIGNEDPAD_WIDTH_
#define	_SIGNEDPAD_LEFT_Y2_		_SIGNEDPAD_LEFT_Y1_	+ _SIGNEDPAD_HIGHT_

#define	_SIGNEDPAD_RIGHT_X1_		_SIGNEDPAD_OUT_RIGHT_X1_+ _LINE_WIDTH_
#define	_SIGNEDPAD_RIGHT_Y1_		_SIGNEDPAD_OUT_RIGHT_Y1_+ _LINE_WIDTH_
#define	_SIGNEDPAD_RIGHT_X2_		_SIGNEDPAD_RIGHT_X1_	+ _SIGNEDPAD_WIDTH_
#define	_SIGNEDPAD_RIGHT_Y2_		_SIGNEDPAD_RIGHT_Y1_	+ _SIGNEDPAD_HIGHT_

/* 沒用到 START */
//#define	_SIGNEDPAD_OUT_LEFT_X2_		_SIGNEDPAD_OUT_LEFT_X1_	+ _SIGNEDPAD_OUT_WIDTH_
//#define	_SIGNEDPAD_OUT_LEFT_Y2_		_SIGNEDPAD_OUT_LEFT_Y1_	+ _SIGNEDPAD_OUT_HIGHT_
//#define	_SIGNEDPAD_OUT_RIGHT_X2_	_SIGNEDPAD_OUT_RIGHT_X1_+ _SIGNEDPAD_OUT_WIDTH_
//#define	_SIGNEDPAD_OUT_RIGHT_Y2_	_SIGNEDPAD_OUT_RIGHT_Y1_+ _SIGNEDPAD_OUT_HIGHT_
/* 沒用到 END */

/* 此為自行定義的結構，用來紀錄簽名板相關數據 */
typedef struct
{
	int	inPosition;		/* 簽名板方向 */
	int	inSigned;		/* 是否已下筆 */
	int	inHasBeenSelect; /* 是否已進行過簽名方向的設定 */
	unsigned long	ulSignTimeStart;	/* 紀錄開始簽名的時間 */
}SIGNPAD_OBJECT;

/* 功能用 */
int inSIGN_TouchSignature_Start(unsigned int uiX ,unsigned int uiY,unsigned int uiWidth ,unsigned int uiHeight,unsigned char *uszBMPFileName,unsigned long ulTimeout);
int inSIGN_TouchSignature_Internal_END(void);
int inSIGN_GetSignatureStatus(unsigned long *ulStatus , unsigned long *ulDuration);
int inSIGN_IsSigned(SIGNPAD_OBJECT *srSignpad);
int inSIGN_BMPConverter_Left(unsigned char *uszInputBMPFileName, unsigned char *uszOutputBMPFileName);
int inSIGN_BMPConverter_Right(unsigned char *uszInputBMPFileName, unsigned char *uszOutputBMPFileName);
int inSIGN_BMPConverter_OneColor(unsigned char *uszInputBMPFileName, unsigned char *uszOutputBMPFileName);
int inSIGN_Text_To_BMP(unsigned long ulwidth, unsigned long ulHeight, unsigned short usX, unsigned short usY, unsigned char *uszString,unsigned short uszFontSize,char *szBMPFileName);
int inSIGN_Text_To_BMPEx(unsigned short usX, unsigned short usY, unsigned char *uszString,unsigned short uszFontSize,char *szBMPFileName);
int inSIGN_Rotate_TextBMP(char *szFileName, int inRotate);
int inSIGN_Display_Black_Back(unsigned short usX, unsigned short usY, unsigned short usXSize, unsigned short usYSize);

/* 流程 */
int inSIGN_TouchSignature_Flow(TRANSACTION_OBJECT *pobTran);
int inSIGN_TouchSignature_Internal(TRANSACTION_OBJECT *pobTran);
int inSIGN_TouchSignature_Internal_START(TRANSACTION_OBJECT *pobTran, SIGNPAD_OBJECT *srSignpad);
int inSIGN_TouchSignature_Internal_END(void);
int inSIGN_TimeoutCheck(SIGNPAD_OBJECT *srSignpad, int inTimeOut, unsigned char uszDispTimeout);
int inSIGN_TimeoutStart(unsigned long *ulRunTime);
int inSIGN_CheckSignature(TRANSACTION_OBJECT *pobTran);
int inSIGN_Check_NOSignature(TRANSACTION_OBJECT *pobTran);

/* 測試用 */
int inSIGN_TouchSignature_Test(void);

/* 富邦新加簽名前檢查功能用 20190214 [SAM] */
int inSIGN_CheckTransSign(TRANSACTION_OBJECT *pobTran);
int inSIGN_Check_Trans_For_Sign(TRANSACTION_OBJECT *pobTran);
int inSIGN_Check_Esc_Upload_Fail_Confirm(TRANSACTION_OBJECT *pobTran);
int inSIGN_Check_Inst_Amount_Confirm(TRANSACTION_OBJECT *pobTran);
int inSIGN_Check_Redeem_Amount_Confirm(TRANSACTION_OBJECT *pobTran);
int inSIGN_Check_Cup_Amount_Confirm(TRANSACTION_OBJECT *pobTran);


/* 横向檢查簽名 20190627 [SAM] */
int inSIGN_CheckSignatureOfHorizontalView(TRANSACTION_OBJECT *pobTran);

void Bmp_Bigger_And_Smaller(BITMAPFILEHEADER head,BITMAPINFOHEADER info);
int inSIGN_TestSignatureView( TRANSACTION_OBJECT *pobTran);
int inSIGN_TestTouchSignature_Internal(TRANSACTION_OBJECT *pobTran);
int inSIGN_TouchSignature_Internal_Calibration(void);
