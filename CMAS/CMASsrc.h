/* 
 * File:   CMASsrc.h
 * Author: Hachi
 *
 * Created on 2020年1月3日, 下午 3:48
 */

#ifndef CMASSRC_H
#define CMASSRC_H

#ifdef __cplusplus
extern "C"
{
#endif


	
int inCMAS_Init_Flow(TRANSACTION_OBJECT *pobTran);
int inCMAS_Func_UpdateTermInvNum(TRANSACTION_OBJECT *pobTran);
int inCMAS_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode);
int inCMASSrc_Display_Review(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Eticket_Initial(TRANSACTION_OBJECT *pobTran);
#ifdef __cplusplus
}
#endif

#endif /* CMASSRC_H */
