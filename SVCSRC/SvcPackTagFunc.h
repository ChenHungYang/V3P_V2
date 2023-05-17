/* 
 * File:   SvcPackTagFunc.h
 * Author: Sam chang
 *
 * Created on 2022年9月19日, 下午 2:10
 */

#ifndef SVCPACKTAGFUNC_H
#define	SVCPACKTAGFUNC_H

#ifdef	__cplusplus
extern "C" {
#endif

int inSVC_PackTagDF66(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF21(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF23(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF26(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF29(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF2B(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF2E(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF2F(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF31(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF3C(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF45(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF4B(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF61(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF65(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);
int inSVC_PackTagFF6E(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf ,int *inPackAddr);



#ifdef	__cplusplus
}
#endif

#endif	/* SVCPACKTAGFUNC_H */

