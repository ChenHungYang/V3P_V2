#ifndef __FILE_LOG_FUNC_H__
#define __FILE_LOG_FUNC_H__

#define _FILE_LOG_NAME_ "FileLog.txt"

#define _FILE_LOG_HANDLE_EXIST_	-2
#define _FILE_LOG_FILE_SUCCESS_	0
#define _FILE_LOG_FILE_ERROR_		-1
#define _FILE_MAX_DATA_			10485760 
#define _FILE_HEADER_DATA_		12	/* 固定標頭 */
#define _FILE_TIME_HEADER_		20	/* 固定傳入時間 */
#define _FILE_FIX_TIME_LENGTH_		172	/* 固定記錄長度 150 + 記錄時間 20 + 0D0A 2BYTE */

int inFILE_LOG_InitLogFile(void);
int inFILE_LOG_CloseLogFile(void);
int inFILE_LOG_ReadLogFile(unsigned char *uszReadBuff, unsigned long ulReadSize);
int inFILE_LOG_WriteLogIntoTheFile(unsigned char *uszWriteBuff, unsigned long ulWriteSize);
int inFILE_LOG_WriteLog(char *szTime, char *szData);
int inFILE_LOG_ReadAndPrintLog(void );
int inFILE_LOG_ReadAndOutputLog(void );

#endif
