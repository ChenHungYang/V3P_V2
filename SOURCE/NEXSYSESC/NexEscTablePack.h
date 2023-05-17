#ifndef _NEXSYS_ESC_PACK_TABLE_
#define _NEXSYS_ESC_PACK_TABLE_

#define _DEFAULT_TABLE_58_LENTH_  20  /* Added Table 58 2017/7/13 */

int inESC_PackTableE1(TRANSACTION_OBJECT *pobTran, unsigned char *unszPackBuf ,int inPackAddr, int inIncode);
int inESC_PackTableE2(TRANSACTION_OBJECT *pobTran, unsigned char *unszPackBuf ,int inPackAddr, int inIncode);
int inESC_PackTableR1(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int inPackAddr, int inIncode);
int inESC_PackTableR2(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int inPackAddr, int inIncode);
int inESC_PackTableR3(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int inPackAddr, int inIncode);
int inESC_PackTableR4(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int inPackAddr, int inIncode);
int inESC_PackTableR5(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int inPackAddr, int inIncode);

int inESC_UnPackTableHeader(TRANSACTION_OBJECT *pobTran, unsigned char *szUnPackBuf, int inPackAddr,int inSubLen);
int inESC_UnPackTable58(TRANSACTION_OBJECT *pobTran, unsigned char *szUnPackBuf, int inPackAddr,int inSubLen);
void vdESC_PackTransType(TRANSACTION_OBJECT *pobTran, char *szPrintBuf1);

int inESC_UuPackTableE1(unsigned char *uszPackBuf , int *inLen , int inUnPackMode);
int inESC_UuPackTableE2(unsigned char *uszPackBuf , int *inLen , int inUnPackMode);
int inESC_UuPackTableR1(unsigned char *uszPackBuf , int *inLen , int inUnPackMode);
int inESC_UuPackTableR2(unsigned char *uszPackBuf , int *inLen , int inUnPackMode);
int inESC_UuPackTableR3(unsigned char *uszPackBuf , int *inLen , int inUnPackMode);
int inESC_UuPackTableR4(unsigned char *uszPackBuf , int *inLen , int inUnPackMode);
int inESC_UuPackTableR5(unsigned char *uszPackBuf , int *inLen , int inUnPackMode);


#endif
