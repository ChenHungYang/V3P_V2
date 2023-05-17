/* 
 * File:   SvcFunc.h
 * Author: Sam chang
 *
 * Created on 2022年9月14日, 上午 11:47
 */

#ifndef SVCFUNC_H
#define	SVCFUNC_H

#ifdef	__cplusplus
extern "C" {
#endif

#define __SVC_TEST_DEF__	
	
int inSVC_GetSTAN(TRANSACTION_OBJECT *pobTran);
int inSVC_SetSTAN(TRANSACTION_OBJECT *pobTran);
int inSVC_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran);
int inSVC_GetSvcCardFieldsFunction_CTLS(TRANSACTION_OBJECT *pobTran);

#ifdef	__cplusplus
}
#endif

#endif	/* SVCFUNC_H */

