#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <unistd.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/Transaction.h"
#include "../../PRINT/Print.h"
#include "../../DISPLAY/Display.h"

#include "../Function.h"

#include "File.h"
#include "FIleLogFunc.h"

static unsigned long st_ulLogFileHandle = -1;
/* 第一行計自內容的總長度 */
static unsigned long st_ulDataAddr = 0;


int inFILE_LOG_InitLogFile(void)
{
	int  inFuncRetVal = _FILE_LOG_FILE_SUCCESS_ ;
	int usRetVal = -1; /* 虹堡API 使用 */
	unsigned long ulFileSize;
	unsigned char unzReadBuff[14], unzCountTemp[15];
		
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
	/* 前10個BYTES 固定為要寫入的位置加上0x0d 0x0a */
	ulFileSize = 12; 
	
	/* 索引為-1 表示未開檔, 檔案及設定只會再開機時設定一次 */
	if(st_ulLogFileHandle == -1)
	{
		
		usRetVal = CTOS_FileOpen((unsigned char *)_FILE_LOG_NAME_, d_STORAGE_FLASH, &st_ulLogFileHandle);

		if (usRetVal != d_OK)
		{
			inDISP_LogPrintfWithFlag(" FileLog Init Open File 2 Ruslt[%d]", usRetVal);
			inFuncRetVal = _FILE_LOG_FILE_ERROR_;
		}else
		{
			inDISP_LogPrintfWithFlag(" FileLog Init Open File Success Handle [%d]", st_ulLogFileHandle);
			
			usRetVal = CTOS_FileGetSize((unsigned char *)_FILE_LOG_NAME_, &ulFileSize);	
			
			if (usRetVal == d_OK){
				/* 如果檔案存在且為12 BYTES 就判斷其長度 */
				if (ulFileSize >= 12 )
				{
					memset(unzReadBuff, 0x00, sizeof(unzReadBuff));
					/* 因為檔案Size 會比我們要讀的檔大，所以固定設為檔頭長度  */
					ulFileSize = 12;
					
					usRetVal = inFILE_LOG_ReadLogFile(unzReadBuff, ulFileSize);

					if(usRetVal == d_OK){
						/* 用來計算長度 */
						memset(unzCountTemp, 0x00, sizeof(unzCountTemp));
						memcpy(unzCountTemp, unzReadBuff, 10 );
						
						st_ulDataAddr = atol((char *)unzCountTemp);
						
						inDISP_LogPrintfWithFlag(" FileLog Init File Seek Addr [%ld]",st_ulDataAddr);
						/* 讀取數值為0 則代表為新檔案 */
						if(st_ulDataAddr == 0){
							
							usRetVal = CTOS_FileSeek(st_ulLogFileHandle, 0,  _SEEK_BEGIN_);
							if(usRetVal != d_OK){
								inDISP_LogPrintfWithFlag(" FileLog Init File Seek 0 *Eerror* in initLog [%ld]",usRetVal);
							}
							
							memset(unzCountTemp, 0x00, sizeof(unzCountTemp));
							memcpy(unzCountTemp, "0000000000\x0d\x0a", _FILE_HEADER_DATA_);
							inFILE_LOG_WriteLogIntoTheFile(unzCountTemp, _FILE_HEADER_DATA_);
							st_ulDataAddr = 0;
						}
	
						/* 讀取資料有長度，則依長度進行設定  */
						/* 因為寫入長度不含檔頭，所以要另外加 _FILE_HEADER_DATA_ */
						usRetVal = CTOS_FileSeek(st_ulLogFileHandle, (st_ulDataAddr +_FILE_HEADER_DATA_), _SEEK_BEGIN_);
						
						if(usRetVal != d_OK){
							inDISP_LogPrintfWithFlag(" FileLog Init File Seek Final Addr *Eerror* in initLog [%ld]",usRetVal);
						}
						
					}else{
						inDISP_LogPrintfWithFlag(" FileLog Init Read File *Error* In OpenFile [%ld]", usRetVal);
						inFuncRetVal = _FILE_LOG_FILE_ERROR_;
					}
			
				}else{
						memset(unzCountTemp, 0x00, sizeof(unzCountTemp));
						/* 因為小於12碼就表示為第一次開檔，或是檔案有問題，檔案設定就重新設定 */
						memcpy(unzCountTemp, "0000000000\x0d\x0a", _FILE_HEADER_DATA_);
						inFILE_LOG_WriteLogIntoTheFile(unzCountTemp, _FILE_HEADER_DATA_);
						st_ulDataAddr = 0;

						/* 因是新的檔案，就把位置指向檔頭後的位置 */
						usRetVal = CTOS_FileSeek(st_ulLogFileHandle, _FILE_HEADER_DATA_, _SEEK_BEGIN_);
						
						if(usRetVal != d_OK){
							inDISP_LogPrintfWithFlag(" FileLog Init Seek < 12  Addr *Eerror* in initLog [%ld]",usRetVal);
						}
						
						inDISP_LogPrintfWithFlag(" FileLog Init Read Data < 12 [%ld]", ulFileSize);
				}
					
			}else{
				inFuncRetVal = _FILE_LOG_FILE_ERROR_;
				inDISP_LogPrintfWithFlag(" FileLog Init CTOS_FileGetSize File *Error* In OpenFile [%ld]", usRetVal);
			}
		}
	}else{
		inFuncRetVal = _FILE_LOG_HANDLE_EXIST_ ;	
		inDISP_LogPrintfWithFlag(" FileLog Init Open File Exist Handle [%d]",st_ulLogFileHandle);
	}
	
	inDISP_LogPrintfWithFlag("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);	
	return inFuncRetVal;
}

int inFILE_LOG_CloseLogFile(void)
{
	unsigned short usRetVal = VS_ERROR ;
	
	/* 檔案索引不為初始值才進行關檔 */
	if(st_ulLogFileHandle != -1)
	{
		/* 關閉檔案Handle */
		usRetVal = CTOS_FileClose(st_ulLogFileHandle);

		if (usRetVal != d_OK)
		{
			inDISP_LogPrintfWithFlag(" FileLog-Close File 2 Ruslt[%d]", usRetVal);
		}else{
			
			st_ulLogFileHandle = -1;
			usRetVal = VS_SUCCESS;
			
			inDISP_LogPrintfWithFlag(" FileLog-Close File Success Handle [%d]",st_ulLogFileHandle);
		}
			
	}else
	{
		inDISP_LogPrintfWithFlag(" FileLog-Close File Handle is Not Ready[%d]", st_ulLogFileHandle);
	}
	return usRetVal;
}


int inFILE_LOG_ReadLogFile(unsigned char *uszReadBuff, unsigned long ulReadSize)
{
	unsigned short usRetVal = VS_ERROR ;
	
	/* 檔案索引不為初始值才進行讀取 */
	if(st_ulLogFileHandle != -1)
	{
		usRetVal = CTOS_FileRead(st_ulLogFileHandle, uszReadBuff, &ulReadSize);
		if (usRetVal != d_OK)
		{
			inDISP_LogPrintfWithFlag(" FileLog-Read File Ruslt[%d]", usRetVal);
		}else{
			usRetVal = VS_SUCCESS;
			inDISP_LogPrintfWithFlag(" FileLog-Read OK  File Len[%ld]", ulReadSize);
		}
		
	}else
	{
		inDISP_LogPrintfWithFlag(" FileLog-Read File Handle Is Not Ready [%d]", st_ulLogFileHandle);
	}
	return usRetVal;
}


int inFILE_LOG_WriteLogIntoTheFile(unsigned char *uszWriteBuff, unsigned long ulWriteSize)
{
	unsigned short usRetVal = VS_ERROR ;
	
	/* 檔案索引不為初始值才進行寫入 */
	if(st_ulLogFileHandle != -1)
	{
		usRetVal = CTOS_FileWrite(st_ulLogFileHandle, uszWriteBuff, ulWriteSize);
		if (usRetVal != d_OK)
		{
			inDISP_LogPrintfWithFlag(" FileLog-Write File 2 Ruslt[%d]", usRetVal);
		}else{
			usRetVal = VS_SUCCESS;
		}
		
	}else
	{
		inDISP_LogPrintfWithFlag("  FileLog-Write File Handle Is Not Ready [%d]", st_ulLogFileHandle);
	}
	return usRetVal;
}

/* 因為需要顯示及寫LOG，所以獨立出來給 inDISP_DispLogAndWriteFlie  ， 不管傳入資料長度多長，
 * 只會抓取 -150.150s  Bytes來使用 */
int inFILE_LOG_WriteLog(char *szTime, char *szData)
{
	int inResult;
	char	szTemplate[20 + 1] = {0};
	char	szDebug2Msg[512 + 1] = {0};
	unsigned long   ulFileSize = 0, usRetVal;

	/* 開機如果沒開檔成功，就不會記錄 */
	if(st_ulLogFileHandle == -1){
		inDISP_LogPrintfWithFlag(" FileLog-WriteLog Not Open Handle [%ld]", st_ulLogFileHandle);
		return VS_SUCCESS;
	}
	
	sprintf(szDebug2Msg,"%s%-150.150s\x0d\x0a", szTime, szData);
	
	usRetVal = CTOS_FileGetSize((unsigned char *)_FILE_LOG_NAME_, &ulFileSize);	
	
	if (usRetVal == d_OK){
		
		if(ulFileSize != (st_ulDataAddr + _FILE_HEADER_DATA_)){
			inDISP_LogPrintfWithFlag(" FileLog-WriteLog stDatalen[%ld] != ulFileSize[%ld] ",  (st_ulDataAddr + _FILE_HEADER_DATA_), ulFileSize);
		}
		
		/* 判斷是否超過檔案限制 ,需加入時間與日期的長度 */
		if( (st_ulDataAddr +_FILE_FIX_TIME_LENGTH_) > _FILE_MAX_DATA_)
		{
			usRetVal = CTOS_FileSeek(st_ulLogFileHandle, _FILE_HEADER_DATA_, _SEEK_BEGIN_);
			st_ulDataAddr = 0;
		}else{
			usRetVal = CTOS_FileSeek(st_ulLogFileHandle, (st_ulDataAddr+ _FILE_HEADER_DATA_), _SEEK_BEGIN_);
		}
			
	}else{
		inDISP_LogPrintfWithFlag(" FileLog-WriteLog Get File Log *Error* [%d] ", usRetVal);
		inFILE_LOG_CloseLogFile();
		return usRetVal;
	}
	
	inResult = inFILE_LOG_WriteLogIntoTheFile((unsigned char*)szDebug2Msg, strlen(szDebug2Msg) );
	
	/* 有寫入，目前檔案位置需要更動 */
	if(inResult == VS_SUCCESS)
	{
		/* 更新加總這次要寫入檔案的檔頭的長度 */
		st_ulDataAddr += strlen(szDebug2Msg);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate,"%010ld", st_ulDataAddr);

		/* 設定位置到檔案開頭  */
		usRetVal = CTOS_FileSeek(st_ulLogFileHandle, 0 , _SEEK_BEGIN_);
		
		if (usRetVal == d_OK)
		{
			if(VS_SUCCESS != inFILE_LOG_WriteLogIntoTheFile((unsigned char*)szTemplate, strlen(szTemplate)))
			{
				inDISP_LogPrintfWithFlag(" FileLog-WriteLog Run inFILE_LOG_WriteLogIntoTheFile *Error* ");
			}
		
		}else{
			inDISP_LogPrintfWithFlag(" FileLog-WriteLog File Seek *Error* [%ld] ", usRetVal);
		}
	}

	return (inResult);
}

int inFILE_LOG_ReadAndPrintLog(void )
{	
	unsigned char *unzReadBuff;
	unsigned short usRetVal;
	unsigned long ulFileSize;
	
	usRetVal = CTOS_FileGetSize((unsigned char*)_FILE_LOG_NAME_, &ulFileSize);
	
	/* 確認檔案是否存在  */
	if (usRetVal == d_OK)
	{
		unzReadBuff = malloc(ulFileSize + 1);
		
		usRetVal = inFILE_LOG_ReadLogFile(unzReadBuff, ulFileSize);
						
		if (usRetVal == VS_SUCCESS)
		{
			CTOS_PrinterPutString(unzReadBuff);
		}else{		
			usRetVal = VS_ERROR;
		}
	
		free(unzReadBuff);
		
	}else{
		inDISP_LogPrintfWithFlag(" Get Size Err in Print Log[%d]", usRetVal );
		usRetVal = VS_ERROR;
	}
	

	return usRetVal;
}


int inFILE_LOG_ReadAndOutputLog(void )
{	
	unsigned char *unzReadBuff;
	unsigned short usRetVal;
	unsigned long ulFileSize,ulMaxReadLen;
	int inHeadLine = 10 ,inEachLineMax = _FILE_FIX_TIME_LENGTH_;
	int inLoop = VS_TRUE;
	
	usRetVal = CTOS_FileGetSize((unsigned char*)_FILE_LOG_NAME_, &ulFileSize);
	
	ulMaxReadLen = inEachLineMax * 50;
	
	/* 確認檔案是否存在  */
	if (usRetVal == d_OK)
	{
		usRetVal = CTOS_FileSeek(st_ulLogFileHandle, 0, _SEEK_BEGIN_);
		
		if (usRetVal != d_OK)
		{
			inDISP_LogPrintfWithFlag(" Output Log Seek File *Error* [0x%04x] Line[%d]", usRetVal, __LINE__ );
		}else
		{
			
			unzReadBuff = malloc( ulMaxReadLen+ 1);
			
			memset(unzReadBuff, 0x00, sizeof(unzReadBuff));
			/* 把檔頊資料讀完  */
			usRetVal = inFILE_LOG_ReadLogFile(unzReadBuff, inHeadLine);
			if (usRetVal != d_OK)
			{
				inDISP_LogPrintfWithFlag(" Output Log Read File *Error* [0x%04x] Line[%d]", usRetVal, __LINE__ );
			}else{
				/*  顯示資料長度 */
				printf((char *)unzReadBuff);
				
				/*  計算需要讀取的資  */
				ulFileSize = ulFileSize - inHeadLine;
				do
				{
					/* 檔案長度比預計長度小，不需要再LOOP讀檔 */
					if(ulFileSize < ulMaxReadLen)
					{
						ulMaxReadLen =  ulFileSize;
						inLoop = VS_ERROR;

					}
					memset(unzReadBuff, 0x00, sizeof(unzReadBuff));

					usRetVal = inFILE_LOG_ReadLogFile(unzReadBuff, ulMaxReadLen);

					if (usRetVal == VS_SUCCESS)
					{
						printf((char *)unzReadBuff);
						ulFileSize -= ulMaxReadLen;
						//inDISP_LogPrintfWithFlag(" File Log Read  ulFileSize[%lu] ulMaxReadLen[%lu]  Line[%d]", ulFileSize,ulMaxReadLen , __LINE__ );
					}else{		
						inDISP_LogPrintfWithFlag(" Output Log Read *Error*  in Output Log usRetVal[0x%04x] ulMaxReadLen[%lu] Line[%d]", usRetVal,ulMaxReadLen , __LINE__ );
						usRetVal = VS_ERROR;
						break;
					}

				}while(inLoop == VS_TRUE );
			}
			
			free(unzReadBuff);
		}

		
	}else{
		inDISP_LogPrintfWithFlag(" Output Log Read Get Size *Error* usRetVal[0x%04x] Line[%d]", usRetVal, __LINE__ );
		usRetVal = VS_ERROR;
	}
	
	
	usRetVal = CTOS_FileSeek(st_ulLogFileHandle, 0, _SEEK_END_);
	if (usRetVal != d_OK)
	{
		inDISP_LogPrintfWithFlag(" Output Log Seek File *Error* usRetVal[0x%04x] Line[%d]", usRetVal, __LINE__ );
	}
	
	return usRetVal;
}

