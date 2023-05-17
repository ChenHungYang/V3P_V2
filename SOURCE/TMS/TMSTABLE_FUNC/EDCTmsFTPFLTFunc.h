#ifndef _EDC_TMS_FTPFLT_FUNC_H_
#define _EDC_TMS_FTPFLT_FUNC_H_

/* 對應到公司TMS文件的 FileList.txt */
typedef struct
{
	char szFTPFileIndex[2 + 1];		/* 檔案索引 */
	char szFTPFileAttribute[2 + 1];	/* 檔案屬性 */
	char szFTPFilePath[60 + 1];		/* 檔案路徑 */
	char szFTPFileSize[10 + 1];		/* 檔案大小 */
} EDC_FTPFLT_REC;


/* 以欄位數決定 ex:欄位數是4個，理論上會有3個comma和兩個byte的0x0D 0X0A */
#define _SIZE_EDC_FTPFLT_COMMA_0D0A_	5
#define _SIZE_EDC_FTPFLT_REC_		(sizeof(EDC_FTPFLT_REC))	/*一個record不含comma和0D0A的長度*/
#define _EDC_FTPFLT_FILE_NAME_		"FTPFLT.dat"			/*file name*/
#define _EDC_FTPFLT_FILE_NAME_BAK_	"FTPFLT.bak"			/*bak name*/


/* Load & Save function */
int inLoadEDCFTPFLTRec(int inFTPFLTRec);
int inSaveEDCFTPFLTRec(int inFTPFLTRec);

/*Set function*/
int inSetEDCFTPFileIndex(char* szFTPFileIndex);
int inSetEDCFTPFileAttribute(char* szFTPFileAttribute);
int inSetEDCFTPFilePath(char* szFTPFilePath);
int inSetEDCFTPFileSize(char* szFTPFileSize);


/*Get function*/
int inGetEDCFTPFileIndex(char* szFTPFileIndex);
int inGetEDCFTPFileAttribute(char* szFTPFileAttribute);
int inGetEDCFTPFilePath(char* szFTPFilePath);
int inGetEDCFTPFileSize(char* szFTPFileSize);


int inEDCTMS_FTPFLT_DeleteAP(char *szFileName);

#endif
