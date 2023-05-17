/* 
 * File:   SvcSrc.h
 * Author: Sam chang
 *
 * Created on 2022年12月12日, 下午 7:12
 */

#ifndef SVCSRC_H
#define	SVCSRC_H

#ifdef	__cplusplus
extern "C" {
#endif


int inSVC_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode);
int inSVC_Tms_Download(TRANSACTION_OBJECT *pobTran);
int inSVC_AutoReload(TRANSACTION_OBJECT *pobTran);
int inSVC_ComparePoint(TRANSACTION_OBJECT *pobTran);

#ifdef	__cplusplus
}
#endif

#endif	/* SVCSRC_H */

