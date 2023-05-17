/* 
 * File:   CMASFunction.h
 * Author: Hachi
 *
 * Created on 2020年1月3日, 下午 3:48
 */

#ifndef CMASFUNCTION_H
#define CMASFUNCTION_H

#ifdef __cplusplus
extern "C"
{
#endif

    int inCMAS_CheckTermStatus(TRANSACTION_OBJECT *pobTran);
    int inCMAS_Decide_Trans_Type(TRANSACTION_OBJECT *pobTran);
    int inCMAS_APIFlow(TRANSACTION_OBJECT *pobTran);

    /* 2020/3/17 下午 6:17 */
    int inCMAS_Batch_Check(char *szECCBatchOut);
    int inCMAS_Batch_Update(void);

    /*2020/1/16 下午 4:48 新增交易類別FLOW*/
    int inCMAS_Deduct_Flow(TRANSACTION_OBJECT *pobTran);
    int inCMAS_Refund_Flow(TRANSACTION_OBJECT *pobTran);
    int inCMAS_TOP_UP_Flow(TRANSACTION_OBJECT *pobTran);
    int inCMAS_Void_TOP_UP_Flow(TRANSACTION_OBJECT *pobTran);
    int inCMAS_Inquiry_Flow(TRANSACTION_OBJECT *pobTran);
    int inCMAS_Settle_Flow(TRANSACTION_OBJECT *pobTran);
    int inCMAS_AutoAdd_Flow(TRANSACTION_OBJECT *pobTran); //2021/4/7 上午 9:23 [Hachi]

    int inCMAS_Set_Update_Batch_Flag(TRANSACTION_OBJECT *pobTran); /*更新Batch Flag*/
    int inCMAS_ResponeCode(TRANSACTION_OBJECT *pobTran);
    int inCMAS_Func_Check_Transaction_Function_Flow(TRANSACTION_OBJECT *pobTran);

    int inCMAS_Delete_Log(void);
    int inCMAS_Func_PrintReceipt_ByBuffer(TRANSACTION_OBJECT *pobTran);

    /*20200620從ESVC 拉過來*/
    int inCMAS_Fast_Tap_Wait(TRANSACTION_OBJECT *pobTran, char *szUID);
    int inCMAS_Get_ParamValue(TRANSACTION_OBJECT *pobTran);
    int inCMAS_Func_Check_Transaction_Function(int inCode);
#ifdef __cplusplus
}
#endif

#endif /* CMASFUNCTION_H */
