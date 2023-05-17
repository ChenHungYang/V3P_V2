#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_clapi.h>
#include <emv_cl.h>
#include <sqlite3.h>

#include <openssl/e_os2.h>
#include <openssl/crypto.h>
#include <openssl/opensslconf.h>
#include <openssl/ossl_typ.h>
#include <openssl/symhacks.h>
#include <openssl/bio.h>
#include <openssl/evp.h>


#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"
#include "../../EVENT/MenuMsg.h"
#include "../../EVENT/Flow.h"
#include "../../DISPLAY/Display.h"
#include "../../DISPLAY/DispMsg.h"
#include "../../PRINT/Print.h"
#include "../../PRINT/PrtMsg.h"
#include "../../COMM/Comm.h"

#include "../UNIT_FUNC/TimeUint.h"
#include "../Function.h"
#include "../FuncTable.h"
#include "../CFGT.h"
#include "../KMS.h"
#include "../EDC.h"

#include "../../KEY/ProcessTmk.h"

#include "../ECR_FUNC/ECR.h"
#include "../ECR_FUNC/RS232.h"

#include "../MULTI_FUNC/MultiFunc.h"
#include "../MULTI_FUNC/MultiHostFunc.h"
#include "../MULTI_FUNC/JsonMultiHostFunc.h"

#include "../ECR_FUNC/ECR.h"
#include "../../TMS/TMSTABLE/TmsCPT.h"

#include "../Accum.h"

#include "Costco.h"

/* 如果連線目的為CostcoPay主機 →fLinkCostcoPayHost == VS_TRUE
 * 與pobTran.fSendToCostcoPayHost 連動使用，因為V3沒有 pstGet_pobTran 所以另存全域變數
 * 連線完記得關掉Flag，避免出錯 */
VS_BOOL         fLinkCostcoPayHost;  

//typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;
int ginCostco_TSPHostIndex = -1;
extern int              ginDebug;
extern int              ginISODebug;

extern MULTI_TABLE      gstMultiOb;	/* [修改外接感應設備] */
extern MULTI_TRANS_TABLE srMultiHostTransTb[];

extern ECR_GASSTATION_GLOBAL_TABLE gsrECR_GASStation_tb;

/* App Name   : inStoreBCDBin
App Builder   : Miyano
App Date&Time : 2022-07-11 11:19:00
App Function  : 儲存BCD&Bin資料
Input Param   : store1,store2為儲存的變數，data為待存資料
Output Param  : VS_SUCCESS
 */
int inStoreBCDBin(char *store1, char *store2, int data) {
    if (data / 16 < 10)
        *store1 = (data / 16) + '0';
    else
        *store1 = (data / 16) + 55;
    if (data % 16 < 10)
        *store2 = (data % 16) + '0';
    else
        *store2 = (data % 16) + 55;

    return (VS_SUCCESS);
}

VS_BOOL vbCheckCostcoCustom(int inVersion) {
    int inResult = VS_FALSE;
    char szCustomerIndicator[3 + 1];
    /*  
            Costco_New  0   // 代表costco全部的客製化
     */
    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("vbCheckCostcoCustom Start");
    }
    inLoadCFGTRec(0);

    memset(szCustomerIndicator, 0x00, sizeof (szCustomerIndicator));
    inGetCustomIndicator(szCustomerIndicator);

    if (inVersion == Costco_New) {
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_018_COSTCO_SIMPLEQ, strlen(_CUSTOMER_INDICATOR_018_COSTCO_SIMPLEQ)) ||
                !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_019_COSTCO_REDEEMQ, strlen(_CUSTOMER_INDICATOR_019_COSTCO_REDEEMQ)) ||
                !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_020_COSTCO_INSTQ, strlen(_CUSTOMER_INDICATOR_020_COSTCO_INSTQ))) {
            inResult = VS_TRUE;
        }
    }

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("vbCheckCostcoCustom End");
    }
    return (inResult);
}

int inCostcoInitial(void) {
    FILE *fp = NULL;
    TSP_Info Tab_TSP_Info;

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("inCostcoInitial Start");
    }

    memset((char *) &(Tab_TSP_Info), 0x00, sizeof (Tab_TSP_Info));
    memcpy(&Tab_TSP_Info.TSP_IP[0], "192.168.3.120", 13);
    memcpy(&Tab_TSP_Info.TSP_Port[0], "8088", 4);

    remove(_File_Costco_TSP_Info_);

    if ((fp = fopen(_File_Costco_TSP_Info_, "w")) == NULL) {
        inDISP_LogPrintf("%s Open Error", _File_Costco_TSP_Info_);
        return (VS_ERROR);
    }
    fwrite(&Tab_TSP_Info, 1, sizeof (Tab_TSP_Info), fp);
    fclose(fp);

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("inCostcoInitial TSP_IP = %s", Tab_TSP_Info.TSP_IP);
        inDISP_LogPrintf("inCostcoInitial TSP_Port = %s", Tab_TSP_Info.TSP_Port);
        inDISP_LogPrintf("inCostcoInitial End");
    }
    
    inECR_DeInitial();
    inSetECRVersion("04");
    inSetECRComPort("COM1");
    inSetMultiComPort1("COM3");
    inSaveEDCRec(0);
    
    inECR_Initial();
    
    return (VS_SUCCESS);
}

int inSetTSP_IP(char* szTSPIPAddress) {
    FILE *fp = NULL;
    TSP_Info Tab_TSP_Info;

    if (strlen(szTSPIPAddress) < 0 || strlen(szTSPIPAddress) > 15) {
        inDISP_LogPrintf("szTSPIPAddress length Error");
        return (VS_ERROR);
    }

    memset((char *) &(Tab_TSP_Info), 0x00, sizeof (Tab_TSP_Info));

    if ((fp = fopen(_File_Costco_TSP_Info_, "r")) == NULL) {
        inDISP_LogPrintf("%s Open Error", _File_Costco_TSP_Info_);
        return (VS_ERROR);
    }
    fread(&Tab_TSP_Info, 1, sizeof (Tab_TSP_Info), fp);
    fclose(fp);

    memset((char *) &(Tab_TSP_Info.TSP_IP), 0x00, sizeof (Tab_TSP_Info.TSP_IP));
    memcpy(&Tab_TSP_Info.TSP_IP[0], szTSPIPAddress, strlen(szTSPIPAddress));

    if ((fp = fopen(_File_Costco_TSP_Info_, "w")) == NULL) {
        inDISP_LogPrintf("%s Open Error", _File_Costco_TSP_Info_);
        return (VS_ERROR);
    }
    fwrite(&Tab_TSP_Info, 1, sizeof (Tab_TSP_Info), fp);
    fclose(fp);

    return VS_SUCCESS;
}

int inSetTSP_Port(char* szTSPPortNum) {
    FILE *fp = NULL;
    TSP_Info Tab_TSP_Info;

    if (strlen(szTSPPortNum) < 0 || strlen(szTSPPortNum) > 5) {
        inDISP_LogPrintf("szTSPPortNum length Error");
        return (VS_ERROR);
    }

    memset((char *) &(Tab_TSP_Info), 0x00, sizeof (Tab_TSP_Info));

    if ((fp = fopen(_File_Costco_TSP_Info_, "r")) == NULL) {
        inDISP_LogPrintf("%s Open Error", _File_Costco_TSP_Info_);
        return (VS_ERROR);
    }
    fread(&Tab_TSP_Info, 1, sizeof (Tab_TSP_Info), fp);
    fclose(fp);

    memset((char *) &(Tab_TSP_Info.TSP_Port), 0x00, sizeof (Tab_TSP_Info.TSP_Port));
    memcpy(&Tab_TSP_Info.TSP_Port[0], szTSPPortNum, strlen(szTSPPortNum));

    if ((fp = fopen(_File_Costco_TSP_Info_, "w")) == NULL) {
        inDISP_LogPrintf("%s Open Error", _File_Costco_TSP_Info_);
        return (VS_ERROR);
    }
    fwrite(&Tab_TSP_Info, 1, sizeof (Tab_TSP_Info), fp);
    fclose(fp);

    return VS_SUCCESS;
}

int inGetTSP_IP(char* szTSPIPAddress) {
    FILE *fp = NULL;
    TSP_Info Tab_TSP_Info;

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("inGetTSP_IP Start");
    }

    memset((char *) &(Tab_TSP_Info), 0x00, sizeof (Tab_TSP_Info));

    if ((fp = fopen(_File_Costco_TSP_Info_, "r")) == NULL) {
        inDISP_LogPrintf("%s Open Error", _File_Costco_TSP_Info_);
        return (VS_ERROR);
    }
    fread(&Tab_TSP_Info, sizeof (Tab_TSP_Info), 1, fp);
    fclose(fp);

    memcpy(&szTSPIPAddress[0], &Tab_TSP_Info.TSP_IP[0], strlen(Tab_TSP_Info.TSP_IP));

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("inGetTSP_IP End");
    }

    return VS_SUCCESS;
}

int inGetTSP_Port(char* szTSPPortNum) {
    FILE *fp = NULL;
    TSP_Info Tab_TSP_Info;

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("inGetTSP_Port Start");
    }

    memset((char *) &(Tab_TSP_Info), 0x00, sizeof (Tab_TSP_Info));

    if ((fp = fopen(_File_Costco_TSP_Info_, "r")) == NULL) {
        inDISP_LogPrintf("%s Open Error", _File_Costco_TSP_Info_);
        return (VS_ERROR);
    }
    fread(&Tab_TSP_Info, 1, sizeof (Tab_TSP_Info), fp);
    fclose(fp);

    memcpy(&szTSPPortNum[0], &Tab_TSP_Info.TSP_Port[0], strlen(Tab_TSP_Info.TSP_Port));

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("inGetTSP_Port End");
    }

    return VS_SUCCESS;
}

int inCostco_Decide_Display_Image(TRANSACTION_OBJECT *pobTran) {
    char szFunEnable[2 + 1];
    char szCustomerIndicator[3 + 1];
    inDISP_ClearAll();

    if (ginDebug == VS_TRUE)
    {
            inDISP_LogPrintf("inCostco_Decide_Display_Image Start !");
    }
    memset(szFunEnable, 0x00, sizeof (szFunEnable));
    inGetStore_Stub_CardNo_Truncate_Enable(szFunEnable);

    memset(szCustomerIndicator, 0x00, sizeof (szCustomerIndicator));
    inGetCustomIndicator(szCustomerIndicator);
    
    inDISP_LogPrintf("pobTran->srTRec.uszESVCTransBit = %d", pobTran->srTRec.uszESVCTransBit);
    inDISP_LogPrintf("srTRec.inCode = %02x", pobTran->srTRec.inCode);
    
    /* 是否是票證交易 */
    if (pobTran->srTRec.uszESVCTransBit == VS_TRUE) {
        if (pobTran->srTRec.inCode == _TICKET_DEDUCT_) {
            inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜購貨交易＞ */
            inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_); /* 請將票卡放置感應區 */
        } else if (pobTran->srTRec.inCode == _TICKET_REFUND_) {
            inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜退貨交易＞ */
            inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_); /* 請將票卡放置感應區 */
        } else if (pobTran->srTRec.inCode == _TICKET_INQUIRY_) {
            inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜餘額查詢＞ */
            inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_); /* 請將票卡放置感應區 */
        } else if (pobTran->srTRec.inCode == _TICKET_TOP_UP_) {
            inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜加值交易＞ */
            inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_); /* 請將票卡放置感應區 */
        } else if (pobTran->srTRec.inCode == _TICKET_VOID_TOP_UP_) {
            inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜加值取消＞ */
            inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_); /* 請將票卡放置感應區 */
        }            /* [新增電票悠遊卡功能]  新增顯示圖片條件 [SAM] 2022/6/21 下午 6:25 START */
        else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_) {
            inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜1.購貨交易＞ */
            inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_); /* 請將票卡放置感應區 */
        } else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_) {
            inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜2.退貨交易＞ */
            inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_); /* 請將票卡放置感應區 */
        } else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_) {
            inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜3.加值交易＞ */
            inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_); /* 請將票卡放置感應區 */
        } else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_) {
            inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜4.取消加值＞ */
            inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_); /* 請將票卡放置感應區 */
        } else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_) {
            inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜6.餘額查詢＞ */
            inDISP_PutGraphic(_CTLS_ESVC_, 0, _COORDINATE_Y_LINE_8_3_); /* 請將票卡放置感應區 */
        }
        /* [新增電票悠遊卡功能]  新增顯示圖片條件 [SAM] 2022/6/21 下午 6:25 END */
    }
    else {
        /* 是否要顯示人工輸入卡號 */
        if (memcmp(szFunEnable, "Y", strlen("Y")) == 0) {
            /* 顯示 請刷銀聯卡或感應 */
            if (pobTran->srBRec.uszCUPTransBit == VS_TRUE) {
                if (pobTran->srBRec.lnHGTransactionType != 0) {
                    if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利積點＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數抵扣＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡加價購＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數兌換＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_INQUIRY_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數查詢＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    } else if (pobTran->srBRec.inCode == _REDEEM_SALE_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利扣抵＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    } else if (pobTran->srBRec.inCode == _INST_SALE_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡分期付款＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    }
                } else {
                    if (pobTran->srBRec.inCode == _CUP_SALE_) {
                        inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯一般交易＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡  */
                    } else if (pobTran->srBRec.inCode == _CUP_REDEEM_SALE_) {
                        inDISP_PutGraphic(_MENU_CUP_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯紅利扣抵＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡  */
                    } else if (pobTran->srBRec.inCode == _CUP_INST_SALE_) {
                        inDISP_PutGraphic(_MENU_CUP_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯分期交易＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡  */
                    } else if (pobTran->srBRec.inCode == _CUP_PRE_AUTH_) {
                        inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡  */
                    } else if (pobTran->srBRec.inCode == _CUP_PRE_COMP_) {
                        inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權完成＞ */
                        inDISP_PutGraphic(_CTLS_TX_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 顯示 請刷銀聯卡或感應 人工輸入請按0 */
                    } else if (pobTran->srBRec.inCode == _CUP_REFUND_) {
                        inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯退貨＞ */
                        inDISP_PutGraphic(_CTLS_TX_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 顯示 請刷銀聯卡或感應 人工輸入請按0 */
                    } else if (pobTran->srBRec.inCode == _CUP_REDEEM_REFUND_) {
                        inDISP_PutGraphic(_MENU_CUP_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯紅利退貨＞ */
                        inDISP_PutGraphic(_CTLS_TX_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 顯示 請刷銀聯卡或感應 人工輸入請按0 */
                    } else if (pobTran->srBRec.inCode == _CUP_INST_REFUND_) {
                        inDISP_PutGraphic(_MENU_CUP_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯分期退貨＞ */
                        inDISP_PutGraphic(_CTLS_TX_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 顯示 請刷銀聯卡或感應 人工輸入請按0 */
                    }
                }

            }                /* 顯示 請刷卡或感應卡片 */
            else {
                if (pobTran->srBRec.lnHGTransactionType != 0) {
                    if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利積點＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數抵扣＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡加價購＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數兌換＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_INQUIRY_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數查詢＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.inCode == _REDEEM_SALE_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利扣抵＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.inCode == _INST_SALE_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡分期付款＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    }
                } else {
                    if (pobTran->srBRec.inCode == _SALE_) {
                        inDISP_PutGraphic(_MENU_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜一般交易＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 刷卡或插卡或感應 掃描按* */
                    } else if (pobTran->srBRec.inCode == _SALE_OFFLINE_) {
                        inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜補登交易＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 刷卡或插卡或感應 掃描按* */
                    } else if (pobTran->srBRec.inCode == _REDEEM_SALE_) {
                        inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜紅利扣抵＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 刷卡或插卡或感應 掃描按* */
                    } else if (pobTran->srBRec.inCode == _INST_SALE_) {
                        inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜分期交易＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 刷卡或插卡或感應 掃描按* */
                    } else if (pobTran->srBRec.inCode == _PRE_AUTH_) {
                        inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 刷卡或插卡或感應 掃描按* */
                    } else if (pobTran->srBRec.inCode == _PRE_COMP_) {
                        inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權完成＞ */
                        inDISP_PutGraphic(_CTLS_TX_MEG_RF_Q, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡或感應卡片 掃描按* 人工輸入請按0 */
                    } else if (pobTran->srBRec.inCode == _REFUND_) {
                        inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜一般退貨＞ */
                        inDISP_PutGraphic(_CTLS_TX_MEG_RF_Q, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡或感應卡片 掃描按* 人工輸入請按0 */
                    } else if (pobTran->srBRec.inCode == _REDEEM_REFUND_) {
                        inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜紅利退貨＞ */
                        inDISP_PutGraphic(_CTLS_TX_MEG_RF_Q, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡或感應卡片 掃描按* 人工輸入請按0 */
                    } else if (pobTran->srBRec.inCode == _INST_REFUND_) {
                        inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜分期退貨＞ */
                        inDISP_PutGraphic(_CTLS_TX_MEG_RF_Q, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡或感應卡片 掃描按* 人工輸入請按0 */
                    } else if (pobTran->srBRec.inCode == _REDEEM_ADJUST_) {
                        inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜紅利調帳＞ */
                        inDISP_PutGraphic(_CTLS_TX_MEG_RF_Q, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡或感應卡片 掃描按* 人工輸入請按0 */
                    } else if (pobTran->srBRec.inCode == _INST_ADJUST_) {
                        inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜分期調帳＞ */
                        inDISP_PutGraphic(_CTLS_TX_MEG_RF_Q, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡或感應卡片 掃描按* 人工輸入請按0 */
                    }
                }

            }
        }            /* 不顯示人工輸入卡號 */
        else {
            /* 顯示 請刷銀聯卡或感應 */
            if (pobTran->srBRec.uszCUPTransBit == VS_TRUE) {
                if (pobTran->srBRec.lnHGTransactionType != 0) {
                    if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利積點＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數抵扣＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡加價購＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數兌換＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_INQUIRY_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數查詢＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    } else if (pobTran->srBRec.inCode == _REDEEM_SALE_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利扣抵＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    } else if (pobTran->srBRec.inCode == _INST_SALE_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡分期付款＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡 */
                    }
                } else {
                    if (pobTran->srBRec.inCode == _CUP_SALE_) {
                        inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯一般交易＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡  */
                    } else if (pobTran->srBRec.inCode == _CUP_REDEEM_SALE_) {
                        inDISP_PutGraphic(_MENU_CUP_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯紅利扣抵＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡  */
                    } else if (pobTran->srBRec.inCode == _CUP_INST_SALE_) {
                        inDISP_PutGraphic(_MENU_CUP_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯分期交易＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡  */
                    } else if (pobTran->srBRec.inCode == _CUP_PRE_AUTH_) {
                        inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_CUPRF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡或感應銀聯卡  */
                    } else if (pobTran->srBRec.inCode == _CUP_PRE_COMP_) {
                        inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權完成＞ */
                        inDISP_PutGraphic(_CTLS_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 顯示 請刷銀聯卡或感應 */
                    } else if (pobTran->srBRec.inCode == _CUP_REFUND_) {
                        inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯退貨＞ */
                        inDISP_PutGraphic(_CTLS_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 顯示 請刷銀聯卡或感應 */
                    } else if (pobTran->srBRec.inCode == _CUP_REDEEM_REFUND_) {
                        inDISP_PutGraphic(_MENU_CUP_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯紅利退貨＞ */
                        inDISP_PutGraphic(_CTLS_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 顯示 請刷銀聯卡或感應 */
                    } else if (pobTran->srBRec.inCode == _CUP_INST_REFUND_) {
                        inDISP_PutGraphic(_MENU_CUP_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜銀聯分期退貨＞ */
                        inDISP_PutGraphic(_CTLS_CUPMEG_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 顯示 請刷銀聯卡或感應 */
                    }
                }

            }                /* 顯示 請刷卡或感應卡片 */
            else {
                if (pobTran->srBRec.lnHGTransactionType != 0) {
                    if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利積點＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數抵扣＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡加價購＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數兌換＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_INQUIRY_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡點數查詢＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_REDEEM_REFUND_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡扣抵退貨＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_REFUND_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡回饋退貨＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.inCode == _REDEEM_SALE_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡紅利扣抵＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    } else if (pobTran->srBRec.inCode == _INST_SALE_) {
                        inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜HG卡分期付款＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡、插卡 或感應卡片 */
                    }
                } else {
                    if (pobTran->srBRec.inCode == _SALE_) {
                        inDISP_PutGraphic(_MENU_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜一般交易＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 刷卡或插卡或感應 掃描按* */
                    } else if (pobTran->srBRec.inCode == _SALE_OFFLINE_) {
                        inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜補登交易＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 刷卡或插卡或感應 掃描按* */
                    } else if (pobTran->srBRec.inCode == _REDEEM_SALE_) {
                        inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜紅利扣抵＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 刷卡或插卡或感應 掃描按* */
                    } else if (pobTran->srBRec.inCode == _INST_SALE_) {
                        inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜分期交易＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 刷卡或插卡或感應 掃描按* */
                    } else if (pobTran->srBRec.inCode == _PRE_AUTH_) {
                        inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權＞ */
                        inDISP_PutGraphic(_CTLS_MEG_ICC_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 刷卡或插卡或感應 掃描按* */
                    } else if (pobTran->srBRec.inCode == _PRE_COMP_) {
                        inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜預先授權完成＞ */
                        inDISP_PutGraphic(_CTLS_MEG_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡或感應卡片 掃碼請按* */
                    } else if (pobTran->srBRec.inCode == _REFUND_) {
                        inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜一般退貨＞ */
                        inDISP_PutGraphic(_CTLS_MEG_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡或感應卡片 掃碼請按* */
                    } else if (pobTran->srBRec.inCode == _REDEEM_REFUND_) {
                        inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜紅利扣抵退貨＞ */
                        inDISP_PutGraphic(_CTLS_MEG_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡或感應卡片 掃碼請按* */
                    } else if (pobTran->srBRec.inCode == _INST_REFUND_) {
                        inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜分期交易退貨＞ */
                        inDISP_PutGraphic(_CTLS_MEG_RF_QR_, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡或感應卡片 掃碼請按* */
                    } else if (pobTran->srBRec.inCode == _REDEEM_ADJUST_) {
                        inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜紅利調帳＞ */
                        inDISP_PutGraphic(_CTLS_TX_MEG_RF_Q, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡或感應卡片 掃描按* 人工輸入請按0 */
                    } else if (pobTran->srBRec.inCode == _INST_ADJUST_) {
                        inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜分期調帳＞ */
                        inDISP_PutGraphic(_CTLS_TX_MEG_RF_Q, 0, _COORDINATE_Y_LINE_8_3_); /* 請刷卡或感應卡片 掃描按* 人工輸入請按0 */
                    }
                }

            }
        }
    }
    
    if (ginDebug == VS_TRUE)
    {
            inDISP_LogPrintf("inCostco_Decide_Display_Image End !");
    }
    
    return (VS_SUCCESS);
}

/* App Name    :inCostco_TSP_Upload_Flow
App Builder   : Miyano
App Date&Time : 2023-01-09 16:05
App Function  : 發送CostcoPay流程
Input Param   : TRANSACTION_OBJECT *pobTran
Output Param  : 1. 成功
                2. 失敗
 */
int inCostco_TSP_Upload_Flow(TRANSACTION_OBJECT *pobTran) {
    //        char    szDispMsg[24] = {0};
    //        char    szTemplate[50 + 1] = {0};
    char szCommmode[1 + 1] = {0};
    char szDemoMode[2 + 1] = {0};
    int inRetVal = 0;
    unsigned long ulRunTime;

    /* 用區域的時間計算 */
    //        inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----",__FILE__, __FUNCTION__, __LINE__);
    inTIME_UNIT_InitCalculateRunTimeGlobal_Start(&ulRunTime, "inCostco_TSP_Upload_Flow Start");

    /* 不是網路就跳過 */
    if (!memcmp(szCommmode, _COMM_MODEM_MODE_, 1)) {
        if (ginDebug == VS_TRUE) {
            inDISP_LogPrintf("MODEM");
        }
        inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inCostco_TSP_Upload_Flow END");
        //            inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
        return VS_SUCCESS;
    }

    /* 不是Costco跳過 */
    if (vbCheckCostcoCustom(Costco_New) != VS_TRUE) {
        return (VS_SUCCESS);
    }

    /* 教育訓練模式 */
    if (memcmp(szDemoMode, "Y", strlen("Y")) == 0) {
        inDISP_ClearAll();
        inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
        inDISP_PutGraphic(_TSP_Upload_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜TSP 上傳＞ */
        inDISP_ChineseFont("請稍候", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
        inDISP_Wait(500);
        inTIME_UNIT_GetRunTimeGlobal(ulRunTime, "inCostco_TSP_Upload_Flow END");
        //            inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] END -----",__FILE__, __FUNCTION__, __LINE__);
        return (VS_SUCCESS);
    }

    /* 一般模式 */
    /* 暫時：不分Online/offline 一律上傳 */
    /* 防呆，把之前交易的連線先斷掉 */
    inCOMM_End(pobTran);

    /* 切換到 TSP HOST */
    /* 找到 TSP HOST 並存 ginCostco_TSPHostIndex 參數並讀取 HDT 不會影響 srBRec.inHDTIndex */
    /* 在這讀取的原因是為了 _COMM_START_ 用 */
    if (inCostco_SwitchToTSP_Host(pobTran->srBRec.inHDTIndex) != VS_SUCCESS) {
        /* 因為不回傳成功會影響後續處理，所以在只影響 TSP 狀況下 先回傳 SUCCESS */
        inDISP_DispLogAndWriteFlie("Switch TSP Host *Error* HDT[%d] Line[%d] ", pobTran->srBRec.inHDTIndex, __LINE__);
        //                inFunc_EDCLock();
        if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR) {
            inDISP_DispLogAndWriteFlie("Switch TSP Host Lod HDT *Error* HDT[%d] Line[%d]", pobTran->srBRec.inHDTIndex, __LINE__);
        }
        return VS_SUCCESS;
    }
    /* 通訊也要切到 TSP 主機(通訊內會直接loadHDT內的CommIndex會自動切過去) */

    inDISP_ClearAll();
    inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
    inDISP_PutGraphic(_TSP_Upload_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜TSP 上傳＞ */
    inDISP_ChineseFont("請稍候", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

    /* 開始連線 */
    pobTran->inActionCode = ACTION_COSTCOPAY_TRANSACTION; /* 設定交易類別 */
    pobTran->inISOTxnCode = _COSTCO_TSP_; /* 封包別為 _COSTCO_TSP_ */
    inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
    if (inRetVal != VS_SUCCESS) {
        /* 通訊失敗，TSP不跳錯誤繼續原交易 */
        //                pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
        //                pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
        //                inFunc_Display_Error(pobTran); /* 通訊失敗 */
        return (VS_SUCCESS);
    }

    /* 連線成功 */
    //	inCostco_TSP_ProcessOnline(pobTran);

    return inRetVal;
}

int inCostco_SwitchToTSP_Host(int inOrgHDTIndex) {
    /* 開機後只找一次 TSP */
    if (ginCostco_TSPHostIndex != -1) {
        /* 直接使用 */
    } else {
        if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_TSP_, &ginCostco_TSPHostIndex) != VS_SUCCESS) {
            /* 找不到直接return VS_ERROR */
            return (VS_ERROR);
        }
    }

    /* 如果Load失敗，回復原Host並回傳VS_ERROR(理論上不會有此情況) */
    if (inLoadHDTRec(ginCostco_TSPHostIndex) < 0) {
        if (inOrgHDTIndex >= 0) {
            /* 回覆原本的Host */
            inLoadHDTRec(inOrgHDTIndex);
        } else {
            /* load回信用卡主機 */
            inLoadHDTRec(0);
        }

        inDISP_DispLogAndWriteFlie("  TSP Switch Host *Error*  TSPHid[%d] Line[%d]", ginCostco_TSPHostIndex, __LINE__);
        return (VS_ERROR);
    } else {
        return (VS_SUCCESS);
    }
}

int inCostco_TSP_ProcessOnline(TRANSACTION_OBJECT *pobTran) {
    int i;
    int inRetVal;
    int inTempHostId = 0;
    char szTemplate[100 + 1];
    COSTCOPAY_OBJECT srPack;

    inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

    memset((char *) &srPack, 0x00, sizeof (COSTCOPAY_OBJECT));
    srPack.inReceTimeOut = 30; /* 設定【收】的 TimeOut */
    
    /* 暫時，For ScanPay(如果別的交易別有需要再慢慢加) */
    srPack.inCode = pobTran->inActionCode;
    
    /* 暫時，注意520 TMSNII 可能用法和V3 TMS NII不一樣 */
    memset(srPack.szNII, 0x00, sizeof (srPack.szNII));
    if(inGetTMSNII(&srPack.szNII[0]) != VS_SUCCESS)
    {
            memcpy(&srPack.szNII[0], "0888", 4);
    }
    inDISP_LogPrintf("Miyano_Test NII = [%s]  ", srPack.szNII);
    /* TPDU */
    memset(srPack.szTPDU, 0x00, sizeof (srPack.szTPDU));
    sprintf(srPack.szTPDU, "60%s0000", srPack.szNII);
    inDISP_LogPrintf("Miyano_Test NII = [%s]  ", srPack.szTPDU);
//    inGetTPDU((char *) srPack.szTPDU);

    while (1) {
        /* 開始組交易封包，送、收、組、解 */
        inDISP_LogPrintf("pobTran->inISOTxnCode = [%d]  ", pobTran->inISOTxnCode);
        inDISP_LogPrintf("pobTran->srBRec.inHDTIndex = [%d]  ", pobTran->srBRec.inHDTIndex);
        
        inRetVal = inCostco_TSP_SendPackRecvUnPack(pobTran, &srPack);

        if (inRetVal != VS_SUCCESS) {

            inDISP_DispLogAndWriteFlie("  TSP SendPack *Error* inRetVal[%d] Line[%d]", inRetVal, __LINE__);
            return (inRetVal);
        }

    }

    inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

    return (VS_SUCCESS);
}

int inCostco_TSP_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran, COSTCOPAY_OBJECT *srPack) {
    int inSendCnt = 0, inRecvCnt = 0, inRetVal;
    unsigned char uszSendPacket[_Costco_TSP_ISO_SEND_ + 1], uszRecvPacket[_Costco_TSP_ISO_RECV_ + 1];

    inDISP_DispLogAndWriteFlie("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

    memset(uszSendPacket, 0x00, sizeof (uszSendPacket));
    memset(uszRecvPacket, 0x00, sizeof (uszRecvPacket));

    /* 組 ISO 電文 */
    if ((inSendCnt = inCostco_TSP_PackISO(pobTran, srPack, (unsigned char *) uszSendPacket)) <= 0) {
        inDISP_DispLogAndWriteFlie("  TSP PackISO *Error* inISOTxnCode[%d] Line[%d]", pobTran->inISOTxnCode, __LINE__);
        return (VS_ERROR); /* 組交易電文錯誤，不用組_REVERSAL_ */
    }

    /* 傳送及接收 ISO 電文 */
    if ((inRecvCnt = inCostco_TSP_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS) {
        inDISP_DispLogAndWriteFlie("  TSP SendRec *Error* inISOTxnCode[%d] uszOfflineBit[%d] Line[%d]", pobTran->inISOTxnCode, pobTran->srBRec.uszOfflineBit, __LINE__);
        if (pobTran->srBRec.uszOfflineBit == VS_TRUE)
            memset(pobTran->srBRec.szRespCode, 0x00, sizeof (pobTran->srBRec.szRespCode));

        return (VS_ERROR);
    }

    /* 解 ISO 電文 */
    inRetVal = inCostco_TSP_UnPackISO(pobTran, srPack, (unsigned char *)uszRecvPacket, inRecvCnt);
    if (inRetVal != VS_SUCCESS) {
        return VS_ERROR;
    }        /* 上傳途中任一封包不接受 */
    else {
        if (memcmp(pobTran->srBRec.szRespCode, "00", 2)) {
            /* 回覆碼不為"00"，表示有問題，先回拒絕 */
            inRetVal = VS_ESCAPE;
        }
    }
    inESC_DebugLog("-----[%s][%s][%d] END -----", __FILE__, __FUNCTION__, __LINE__);
    return (inRetVal);
}

int inCostco_TSP_PackISO(TRANSACTION_OBJECT *pobTran, COSTCOPAY_OBJECT *srPack, unsigned char *uszSendBuf) {
    int inCnt = 0;
    int inPacketCnt = 0;
    int inObjectLen = 0;
    unsigned char uszLRC;
    unsigned char szTemplate[100], szPacket[6000 + 1];
    unsigned char szActionMessage[6000 + 1];
    unsigned char szObject[6000 + 1];

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("inCostco_TSP_PackISO Start");
    }

    memset(szPacket, 0x00, sizeof (szPacket));
    
    /* Even though multipart does fragmenting the requested file content into multiple responses,
    client does not need to make multiple requests to get all the responses.*/

    if (srPack->fMultiResp == VS_TRUE)
            return (VS_RECE_CONTINUE);

    /* TAG_21 Action tag, length, and value. */
    if (srPack->inCode != ACTION_KEEP_ALIVE && srPack->inCode != ACTION_UPDATE_TRANSACTION && srPack->inCode != ACTION_COSTCOPAY_TRANSACTION)
    {
            szPacket[inPacketCnt ++] = 0x21;
            szPacket[inPacketCnt ++] = 0x04;

            switch (srPack->inCode)
            {
                    case ACTION_UPDATE_TRANSACTION      :memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x01", 4); break;
                    case ACTION_CHECK_UPDATE_STATUS     :memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x03", 4); break;
                    case ACTION_DOWNLOAD_BINARY         :memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x04", 4); break;
                    case ACTION_DOWNLOAD_PARAMETER      :memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x05", 4); break;
                    case ACTION_SATUS_REPORT            :memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x06", 4); break;
                    case ACTION_DIAGNOSTIC_REPORT       :memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x07", 4); break;
                    case ACTION_DOWNLOAD_BLACKLIST      :memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x08", 4); break;
                    case ACTION_DETOKEN_TRANSACTION     :memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x09", 4); break;
                    case ACTION_KEEP_ALIVE              :break;
                    default                             : break;
            }
            inPacketCnt += 4;
    }
    
    /* TAG_01 VendorID */
    szPacket[inPacketCnt++] = 0x01;
    szPacket[inPacketCnt++] = 0x06;
    memcpy(&szPacket[inPacketCnt], "NEXSYS", 6);
    inPacketCnt += 6; /* 會影響key 請設定nexsys */

    /* TAG_02 TerminalModel */
    szPacket[inPacketCnt++] = 0x02;
    szPacket[inPacketCnt++] = 0x02;
    memcpy(&szPacket[inPacketCnt], "V3", 2);
    inPacketCnt += 2;
    
    /* TAG03_Terminal Variant */
    if (srPack->inCode != ACTION_KEEP_ALIVE && srPack->inCode != ACTION_COSTCOPAY_TRANSACTION )
    {
            szPacket[inPacketCnt ++] = 0x03;
            szPacket[inPacketCnt ++] = 0x00;
    }
    
    /* TAG_04 TerminalSerialNo */
    memset(szTemplate, 0x00, sizeof (szTemplate));
    inFunc_GetSeriaNumber(szTemplate);
    szPacket[inPacketCnt++] = 0x04;
    szPacket[inPacketCnt++] = 0x10; /* Hex : length = 16 */
    memcpy(&szPacket[inPacketCnt], szTemplate, 16);
    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("Miyano test SN = %s", szTemplate);
    }
    inPacketCnt += 16;

    /* TAG_05 MerchantIdentifier */
    memset(szTemplate, 0x00, sizeof (szTemplate));
    inGetMerchantID(szTemplate);
    szPacket[inPacketCnt++] = 0x05;
    szPacket[inPacketCnt++] = strlen(szTemplate);
    memcpy(&szPacket[inPacketCnt], szTemplate, strlen(szTemplate));
    inPacketCnt += strlen(szTemplate);
    inDISP_LogPrintf("Miyano test MerchantID = %s", szTemplate);

    /* TAG_06 TerminalIdentifier */
    if (pobTran->srBRec.fScanPayForSS == VS_TRUE)
    {
            memset(szTemplate, 0x00, sizeof (szTemplate));
            inGetTerminalID(szTemplate);
            vdGetTSP_TID_Data(szTemplate, strlen(szTemplate)); /* 需經處理 */
            szPacket[inPacketCnt++] = 0x06;
            szPacket[inPacketCnt++] = strlen(szTemplate);
            memcpy(&szPacket[inPacketCnt], szTemplate, strlen(szTemplate));
            inPacketCnt += strlen(szTemplate);
            inDISP_LogPrintf("Miyano test TID1 = %s", szTemplate);
    }
    else
    {
            memset(szTemplate, 0x00, sizeof (szTemplate));
            inGetTerminalID(szTemplate);
            szPacket[inPacketCnt ++] = 0x06;
            szPacket[inPacketCnt ++] = strlen(szTemplate);
            memcpy(&szPacket[inPacketCnt], szTemplate, strlen(szTemplate));
            inPacketCnt += strlen(szTemplate);
            inDISP_LogPrintf("Miyano test TID2 = %s", szTemplate);
    }

    /* TAG_07 APIVersion */
    szPacket[inPacketCnt++] = 0x07;
    szPacket[inPacketCnt++] = 0x04;
    memcpy(&szPacket[inPacketCnt], "\x00\x00\x27\x10", 4);
    inPacketCnt += 4;
    inDISP_LogPrintf("Miyano test APIVersion");

    /* TAG_08 Algorithm */
    szPacket[inPacketCnt++] = 0x08;
    szPacket[inPacketCnt++] = 0x01;
    szPacket[inPacketCnt++] = 0x01;
    inDISP_LogPrintf("Miyano test Algorithm");

    /* TSP 特別要包的 */
    if (srPack->inCode == ACTION_UPDATE_TRANSACTION)
    {
        memset(szObject, 0x00, sizeof(szObject));
	inObjectLen = 0;
	inObjectLen = inCostco_Pack_Object_CostcoPay_UpdateTransaction(pobTran, szObject);
	inDISP_LogPrintf("inCostco_TSP_PackISO()_Update_Trans_Sign_Len(%d)", inObjectLen);

	memcpy(&szPacket[inPacketCnt], &szObject[0], inObjectLen);
	inPacketCnt += inObjectLen;
	inDISP_LogPrintf("inCostco_TSP_PackISO()_Update_Trans_All_Len(%d)", inPacketCnt);
    }
    else if (srPack->inCode == ACTION_COSTCOPAY_TRANSACTION)
    {
        memset(szObject, 0x00, sizeof (szObject));
        inObjectLen = 0;
        inObjectLen = inCostco_Pack_Object_CostcoPay_UpdateTransaction(pobTran, szObject);
        inDISP_LogPrintf("inCostco_TSP_PackISO()_CostcoPay_Update_Trans_Len(%d)", inObjectLen);

        memcpy(&szPacket[inPacketCnt], &szObject[0], inObjectLen);
        inPacketCnt += inObjectLen;
        inDISP_LogPrintf("inCostco_TSP_PackISO()_CostcoPay_Update_Trans_All_Len(%d)", inPacketCnt);
    }
    else if (srPack->inCode == ACTION_KEEP_ALIVE)
    {
        /* TAG_21 Action tag, length, and value. */
        szPacket[inPacketCnt ++] = 0x21;
        szPacket[inPacketCnt ++] = 0x04;
        memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x02", 4);
        inPacketCnt += 4;
    }
    else if (srPack->inCode == ACTION_CHECK_UPDATE_STATUS)
    {
        
    }
    else if (srPack->inCode == ACTION_DOWNLOAD_BINARY)
    {
        
    }
    else if (srPack->inCode == ACTION_DOWNLOAD_PARAMETER)
    {
        
    }
    else if (srPack->inCode == ACTION_SATUS_REPORT)
    {
        
    }
    else if (srPack->inCode == ACTION_DIAGNOSTIC_REPORT)
    {
        
    }
    else if (srPack->inCode == ACTION_DOWNLOAD_BLACKLIST)
    {
        
    }
    else if (srPack->inCode == ACTION_DETOKEN_TRANSACTION)
    {
        memset(szObject, 0x00, sizeof(szObject));
        inObjectLen = 0;
        inObjectLen = inCostco_Pack_Object_DetokenTransaction(pobTran, szObject);
        inDISP_LogPrintf("inCostco_TSP_PackISO()_ACTION_DETOKEN_TRANSACTION_Len(%d)", inObjectLen);

        memcpy(&szPacket[inPacketCnt], &szObject[0], inObjectLen);
        inPacketCnt += inObjectLen;
        inDISP_LogPrintf("inCostco_TSP_PackISO()_ACTION_DETOKEN_TRANSACTION_All_Len(%d)", inPacketCnt);
    }
    else
    {
        inDISP_LogPrintf("inCostco_TSP_PackISO()_[inCode = %d]_ERROR", srPack->inCode);
        return (VS_ERROR);
    }

    /* Packet Header & Data Length */
    memset(szActionMessage, 0x00, sizeof (szActionMessage));
    /* Pack Header */
    szActionMessage[inCnt++] = 0x55;
    /* Pack Data Length */
    if (inPacketCnt < 128) /* 0x7f */ {
        szActionMessage[inCnt++] = inPacketCnt;
    } else if (inPacketCnt < 256) /* 0xff */ {
        szActionMessage[inCnt++] = 0x81;
        szActionMessage[inCnt++] = inPacketCnt & 0xff;
    } else if (inPacketCnt < 65536) {
        szActionMessage[inCnt++] = 0x82;
        szActionMessage[inCnt++] = ((inPacketCnt >> 8) & 0xff);
        szActionMessage[inCnt++] = (inPacketCnt & 0xff);
    }

    /* Packet Data */
    memcpy((char *) &szActionMessage[inCnt], &szPacket[0], inPacketCnt);
    inCnt += inPacketCnt;

    /* checksum of the data */
    uszLRC = (unsigned char) CAL_LRC(szActionMessage);
    szActionMessage[inCnt++] = uszLRC;

    memcpy(&uszSendBuf[0], szActionMessage, inCnt);

    /* 暫時不寫ISODebug */
    //	if (ginISODebug == VS_TRUE)
    //        {
    //		vdCB_ATS_ISOFormatDebug_TLV(pobTran, (unsigned char *)uszSendBuf, inCnt);
    //        }

    vdCostco_TSP_PrintHexToString1(uszSendBuf, inCnt);

    inDISP_LogPrintf("inCostco_TSP_PackISO()_[LENGTH = %d]_END", inCnt);
    return (inCnt);
}

void vdGetTSP_TID_Data(char *szTID, int inLen) {
    int i;
    char a[10][10] = {
        {'0', 'A'},
        {'1', 'B'},
        {'2', 'C'},
        {'3', 'D'},
        {'4', 'E'},
        {'5', 'F'},
        {'6', 'G'},
        {'7', 'H'},
        {'8', 'I'},
        {'9', 'J'}
    };

    for (i = 0; i < 10; i++) {
        if (szTID[inLen - 1] == a[i][0]) {
            szTID[inLen - 1] = a[i][1];
            return;
        }
    }

    return;
}

int inCostco_Pack_Object_CostcoPay_UpdateTransaction(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf) {
    int inCnt = 0;
    int i, inPacketCnt = 0;
    int inIssuerIndex;
    int inRetVal = -1;
    char szTermonalSN[9 + 1];
    char szRandom[29 + 1];
    char szTemplate[32 + 1], szPacket[8192 + 1]; /* 修正簽名檔size宣告太小的問題,導致多筆電簽交易鎖機的問題 */
    char szASCII[4 + 1], szBCD[2 + 1];
    char szCRC[1 + 1];
    char szAuthCode[6 + 1];
    char szISOSendData[ATS_ISO_SEND + 1];
    char szISOReceData[ATS_ISO_RECE + 1];
    char szEncryptData[ATS_ISO_SEND + 1];

    unsigned char uszLRC;
    int inEncryptLen = 0;
    /*----------------*/
    int inFHandle = -1;
    int inImageLen = 0;
    char szDispMsg[DISP_STR_LEN + 1];
    char szImage[6000 + 1];
    /*----------------*/

    /* 產生TMK */
    inCostco_TSP_Generate_TerminalMasterKey(pobTran);

    /* 產生TK */
    inCostco_TSP_Generate_TransactionKey(pobTran, D_Default);
    inDISP_LogPrintf("ERM Trans CRC(%02x)", pobTran->szTransCRC);

    /* TAG_09 Checksum for Transaction */
    szPacket[inPacketCnt++] = 0x09;
    szPacket[inPacketCnt++] = 0x01;
    szPacket[inPacketCnt++] = pobTran->szTransCRC;

    /* TAG_0A Random Bytes for Transaction Key*/
    szPacket[inPacketCnt++] = 0x0a;
    szPacket[inPacketCnt++] = 0x1d;
    memcpy(&szPacket[inPacketCnt], &pobTran->szRandom[0], 29);
    inPacketCnt += 29;

    /* TAG_21 */
    szPacket[inPacketCnt++] = 0x21;
    szPacket[inPacketCnt++] = 0x04;
    memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x01", 4);
    inPacketCnt += 4;

    /* TAG41 TransactionRequestData */
    szPacket[inPacketCnt++] = 0x41;

    inEncryptLen = 0;
    memset(szISOSendData, 0x00, sizeof (szISOSendData));
    memset(szEncryptData, 0x00, sizeof (szEncryptData));
    memcpy(&szISOSendData[0], &pobTran->szTransSendData[0], pobTran->inTransSendLen);

    inEncryptLen = inCostco_TSP_AES_DataEncrypt((unsigned char *) pobTran->szTransKey, (unsigned char *) szISOSendData, pobTran->inTransSendLen, (unsigned char *) szEncryptData);

    if (inEncryptLen < 128) {
        szPacket[inPacketCnt++] = inEncryptLen;
    } else if (inEncryptLen < 256) {
        szPacket[inPacketCnt++] = 0x81;
        szPacket[inPacketCnt++] = inEncryptLen;
    } else if (inEncryptLen < 65536) {
        szPacket[inPacketCnt++] = 0x82;
        szPacket[inPacketCnt++] = ((inEncryptLen >> 8) & 0xff);
        szPacket[inPacketCnt++] = (inEncryptLen & 0xff);
    }

    memcpy(&szPacket[inPacketCnt], szEncryptData, inEncryptLen);
    inPacketCnt += inEncryptLen;
    inDISP_LogPrintf("ERM Trans Encrypt Send Len(%d)", inEncryptLen);

    /* TAG42 Transaction Response Data */
    if (pobTran->srBRec.uszOfflineBit != VS_TRUE) {
        szPacket[inPacketCnt++] = 0x42;

        inEncryptLen = 0;
        memset(szISOReceData, 0x00, sizeof (szISOReceData));
        memset(szEncryptData, 0x00, sizeof (szEncryptData));
        memcpy(&szISOReceData[0], &pobTran->szTransReceData[0], pobTran->inTransReceLen);

        inEncryptLen = inCostco_TSP_AES_DataEncrypt((unsigned char *) pobTran->szTransKey, (unsigned char *) szISOReceData, pobTran->inTransReceLen, (unsigned char *) szEncryptData);

        if (inEncryptLen < 128) {
            szPacket[inPacketCnt++] = inEncryptLen;
        } else if (inEncryptLen < 256) {
            szPacket[inPacketCnt++] = 0x81;
            szPacket[inPacketCnt++] = inEncryptLen;
        } else if (inEncryptLen < 65536) {
            szPacket[inPacketCnt++] = 0x82;
            szPacket[inPacketCnt++] = ((inEncryptLen >> 8) & 0xff);
            szPacket[inPacketCnt++] = (inEncryptLen & 0xff);
        }

        memcpy(&szPacket[inPacketCnt], szEncryptData, inEncryptLen);
        inPacketCnt += inEncryptLen;
        inDISP_LogPrintf("ERM Trans Encrypt Rece Len(%d)", inEncryptLen);
    }

    /* TAG45 CVM Type */
    szPacket[inPacketCnt++] = 0x45;
    szPacket[inPacketCnt++] = 0x04;

    if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
        memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x01", 4);
    else {
        switch (atoi(pobTran->srBRec.inSignStatus)) {
            case 1: memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x01", 4);
                break;
            case 2: memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x02", 4);
                break;
            case 3: memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x03", 4);
                break;
            case 4: memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x04", 4);
                break;
            case 5: memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x05", 4);
                break;
            case 6: memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x06", 4);
                break;
            case 7: memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x07", 4);
                break;
            case 8: memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x08", 4);
                break;
            default: memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x00", 4);
                break;
        }
    }

    inPacketCnt += 4;

    /* TAG46 Card Type */
    szPacket[inPacketCnt++] = 0x46;
    szPacket[inPacketCnt++] = 0x04;
    {
        inIssuerIndex = atoi(pobTran->srBRec.szIssueNumber);
        //		if (inLoadIITRec(inIssuerIndex) < 0)
        //			return (VS_ERROR);

        for (i = 0;; i++) {
            if (inLoadCDTRec(i) < 0) /* 卡別參數檔 */
                return (VS_ERROR);

            memset(szTemplate, 0x00, sizeof (szTemplate));
            inGetCardLabel(szTemplate);

            if (!memcmp(szTemplate, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_))) {
                memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x00", 4);
                break;
            } else if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) {
                memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x00", 4);
                break;
            } else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) {
                memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x01", 4);
                break;
            } else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_))) {
                memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x03", 4);
                break;
            } else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_))) {
                memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x02", 4);
                break;
            } else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_))) {
                memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x07", 4);
                break;
            } else if (!memcmp(szTemplate, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_))) {
                memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x10", 4);
                break;
            } else if (!memcmp(szTemplate, _CARD_TYPE_SMARTPAY_, strlen(_CARD_TYPE_SMARTPAY_))) {
                memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x01", 4);
                break;
            } else {
                memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x01", 4);
                break;
            }
        }
    }
    inPacketCnt += 4;

    /* TAG47 Batch No */
    szPacket[inPacketCnt++] = 0x47;
    szPacket[inPacketCnt++] = 0x06;
    sprintf((char *) &szPacket[inPacketCnt], "%06ld", inGetBatchNum());
    inPacketCnt += 6;

    /* TAG48 Referral Approval Code */
    szPacket[inPacketCnt++] = 0x48;
    szPacket[inPacketCnt++] = 0x06;

    memset(szAuthCode, 0x00, sizeof (szAuthCode));
    strcpy(szAuthCode, pobTran->srBRec.szAuthCode);
    inFunc_PAD_ASCII(szAuthCode, szAuthCode, ' ', 6, _PAD_LEFT_FILL_RIGHT_);
    memcpy((char *) &szPacket[inPacketCnt], &szAuthCode[0], 6);
    inPacketCnt += 6;

    /* TAG49 TSP Transaction ID */
    szPacket[inPacketCnt++] = 0x49;
    szPacket[inPacketCnt++] = strlen(pobTran->srBRec.szTSPTransactionID);
    memcpy((char *) &szPacket[inPacketCnt], pobTran->srBRec.szTSPTransactionID, strlen(pobTran->srBRec.szTSPTransactionID));
    inPacketCnt += strlen(pobTran->srBRec.szTSPTransactionID);

    inCnt = inPacketCnt;
    memcpy(&uszPackBuf[0], &szPacket[0], inCnt);

    inDISP_LogPrintf("inCostco_Pack_Object_CostcoPay_UpdateTransaction(%ld) END!!", inCnt);
    return (inCnt);
}

int inCostco_TSP_Generate_TerminalMasterKey(TRANSACTION_OBJECT *pobTran) {
    int inRetVal;
    char szSerialNo[16 + 1];
    unsigned char szVendorID[8 + 1];
    unsigned char szTerminalModel[16 + 1];
    unsigned char szSalt[50 + 1];
    unsigned char szTMK[128 + 1];
    unsigned char szVMK[32 + 1];

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("inCostco_TSP_Generate_TerminalMasterKey Start");
    }

    /* 20210707,浩瑋修改,key會隨著CUPMODE的值來決定正式或測試*/
    inRetVal = inCostco_Check_Key();
    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("inCostco_Check_Key = %d", inRetVal);
    }
    if (inRetVal == _KeyTest_Product_) {
        memset(szVMK, 0, sizeof (szVMK));
        memcpy(szVMK, "\xb0\xe8\xd5\xcc\xe8\x6c\xac\xb6\x08\xc5\xf1\xa6\x46\x6e\x9e\xa2\x65\xc1\xa9\x4b\xd1\x2d\xe6\xde\x1f\x99\x14\x79\x90\x9b\x83\xaa", 32);
    } else if (inRetVal == _KeyTest_Test_) {
        memset(szVMK, 0, sizeof (szVMK));
        memcpy(szVMK, "\xbc\x17\x3d\x0d\x53\x46\x0c\xff\x2d\x82\xdc\xc2\xa9\xef\x50\x63\xfa\x57\xec\x74\xd6\x4f\xff\xd8\x1b\x5a\x19\x13\x4b\x18\xe9\x11", 32);
    } else {
        return VS_ERROR;
    }

    /*
    =================
    Integration Property (Development environment)
    =================
    VENDOR_ID: NEXSYS
    Vendor Master Key (VMK): 54f6d87fd8556131e1e379a9f2c8852a455c0df7b65fe9c1784b93518cd6ebda
    Initial Vector (IV): a583b41b1b5a4bf2c70cf9dfedc62cbc

    VENDOR ID is all capital letter. and the VMK is generated base on VENDOR ID, if you wish to change your VENDOR ID,
    please file us a request, we will generate development VMK for you.
    For production key, it will provided during integration environment, as production key required bank user to generate.

    Any question, please contact: cub-support-team@softspace.com.my */

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("inCostco_TSP_Generate_TerminalMasterKey Start");
    }

    memset(szVendorID, 0x00, sizeof (szVendorID));
    memset(szTerminalModel, 0x00, sizeof (szTerminalModel));
    memset(szSerialNo, 0x00, sizeof (szSerialNo));
    memset(szSalt, 0xFF, sizeof (szSalt));

    memcpy(&szVendorID[0], "NEXSYS", 6); /* \x41\x45\x58\x53\x59\x53 */
    memcpy(&szTerminalModel[0], "V3", 2);

    inFunc_GetSeriaNumber(szSerialNo);

    sprintf((char *) &szSalt[0], "%s|%s|%s", szVendorID, szTerminalModel, szSerialNo);

    memset(szTMK, 0x00, sizeof (szTMK));
    /* Vendor Master key + (VerdorID|TerminalModel|SN) = TMK */
    inCostco_TSP_AES_Encryption(szVMK, szSalt, szTMK);

    if (1) {
        inDISP_LogPrintf("Vendor Master key = ");
        vdCostco_TSP_PrintHexToString(szVMK);
        inDISP_LogPrintf("Terminal Master key = ");
        vdCostco_TSP_PrintHexToString(szTMK);
    }

    /* 20230113 Miyano 下面兩行先保留，不知道幹嘛 */
    memset(pobTran->szTerminalMasterKey, 0x00, sizeof (pobTran->szTerminalMasterKey));
    memcpy(&pobTran->szTerminalMasterKey[0], &szTMK[0], 32);

    inDISP_LogPrintf("inCostco_TSP_Generate_TerminalMasterKey()_END");

    return (VS_SUCCESS);
}

/*
 * 20230113 Miyano add 
 * 檢查當前Key是正式key還是測試key
 * 正式Key Return _KeyTest_Product_ (1)
 * 測試Key Return _KeyTest_Test_    (0)
 * 找不到Key或有問題 Return -1(VS_Error)
 * 不確定是否為Costco要的，待確認
 */
int inCostco_Check_Key(void) {
    int inRetVal;
    int inCVLen;
    int inHashAlgorithm;
    char szKeyCheckValue[6 + 1];
    char szTest;
    unsigned short usKeySet;
    unsigned short usKeyIndex;
    CTOS_KMS2KEYGETINFO_PARA srPara;

    /* 預設Key 位置 */
    usKeySet = _TMK_KEYSET_NCCC_;
    usKeyIndex = _TMK_KEYINDEX_NCCC_;

    /* CHeckValueLen
     * Only used for KeyType 3DES/3DES-DUKPT/AES
     */
    inCVLen = 6;

    /* HashAlgorithm
     * Only used for KeyType RSA
     * SHA1		KMS2_KEYCERTIFICATEGENERATECIHERMETHOD_DEFAULT_WITH_SHA1 (0x00)
     * SHA256	KMS2_KEYCERTIFICATEGENERATECIHERMETHOD_DEFAULT_WITH_SHA2 (0x01)
     */
    inHashAlgorithm = 0x00;

    /* 仿inNCCC_TMK_GetKeyInfo_LookUp_Default 保留迴圈架構，但這裡只確認第一把Key就好 */
    for (usKeyIndex = _TMK_KEYINDEX_NCCC_; usKeyIndex < _TMK_KEYINDEX_NCCC_ + _KEY_TOTAL_COUNT_; usKeyIndex++) {
        /* 放入結構中 */
        memset(&srPara, 0x00, sizeof (CTOS_KMS2KEYGETINFO_PARA));
        srPara.Version = 0x01;
        srPara.Input.KeySet = usKeySet;
        srPara.Input.KeyIndex = usKeyIndex;
        srPara.Input.CVLen = inCVLen; /* Only used for KeyType 3DES/3DES-DUKPT/AES */
        srPara.Input.HashAlgorithm = inHashAlgorithm; /* Only used for KeyType RSA */
        srPara.Output.pCV = (unsigned char*) szKeyCheckValue;

        inRetVal = inKMS_GetKeyInfo(&srPara);

        if (inRetVal == VS_SUCCESS) /* 成功 */ {
            szTest = 0x00;
            inNCCC_TMK_Check_Test_Key(srPara.Input.KeySet, srPara.Input.KeyIndex, &szTest);
            if (szTest == 'Y') /* 測試Key */ {
                if (ginDebug == VS_TRUE) {
                    inDISP_LogPrintf("Test Key");
                }
                inRetVal = _KeyTest_Test_;
            } else /* 正式Key */ {
                if (ginDebug == VS_TRUE) {
                    inDISP_LogPrintf("Product Key");
                }
                inRetVal = _KeyTest_Product_;
            }
        } else if (inRetVal == d_KMS2_KEY_NOT_EXIST) {
            inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
            inDISP_ChineseFont_Color("該位置Key不存在", _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_RED_, _DISP_LEFT_);
            uszKBD_GetKey(30);
            inRetVal = VS_ERROR;
        } else {
            /* 失敗 */
            inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
            inDISP_ChineseFont_Color("其他錯誤", _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_RED_, _DISP_LEFT_);
            uszKBD_GetKey(30);
            inRetVal = VS_ERROR;
        }
    }

    return inRetVal;
}

int inCostco_TSP_AES_Encryption(unsigned char *szKey, unsigned char *szPlainText, unsigned char *szCipherText) {
    EVP_CIPHER_CTX ctx;

    int i, inNumPad = 0;
    int inLen = 0;
    int inPlainTextLen = 0;
    int inCipherTextLen = 0;
    int inCipherBlockSize, inCipherKeyLength, inCipherIvLength;

    unsigned char szIV[] ={/* Only need 16 bytes... */
        0xa5, 0x83, 0xb4, 0x1b, 0x1b, 0x5a, 0x4b, 0xf2,
        0xc7, 0x0c, 0xf9, 0xdf, 0xed, 0xc6, 0x2c, 0xbc, 0
    };

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("inCostco_TSP_AES_Encryption Start");
    }

    inPlainTextLen = strlen((const char *) szPlainText);
    inDISP_LogPrintf("inCostco_TSP_AES_Encryption Len =[%d]", inPlainTextLen);

    /* PKCS */
    inNumPad = 16 - (inPlainTextLen % 16); /* 計算需要Padding數量 */

    for (i = 0; i < inNumPad; i++)
        szPlainText[inPlainTextLen + i] = inNumPad;

    inPlainTextLen = inPlainTextLen + inNumPad;

    vdCostco_TSP_PrintHexToString1(szPlainText, inPlainTextLen);

    EVP_CIPHER_CTX_init(&ctx);
    EVP_EncryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, (const unsigned char *) szKey, (const unsigned char *) szIV);

    /* 關閉padding 功能,預設開啟 */
    EVP_CIPHER_CTX_set_padding(&ctx, 0);

    inCipherBlockSize = EVP_CIPHER_CTX_block_size(&ctx);
    inCipherKeyLength = EVP_CIPHER_CTX_key_length(&ctx);
    inCipherIvLength = EVP_CIPHER_CTX_iv_length(&ctx);

    inDISP_LogPrintf("INFO(evp_encrypt): Input length: %d", inPlainTextLen);
    inDISP_LogPrintf("INFO(evp_encrypt): block size: %d", inCipherBlockSize);
    inDISP_LogPrintf("INFO(evp_encrypt): key length: %d", inCipherKeyLength);
    inDISP_LogPrintf("INFO(evp_encrypt): IV length:  %d", inCipherIvLength);

    if ((inCipherKeyLength > 32) || (inCipherIvLength > 16)) {
        inDISP_LogPrintf("inCostco_TSP_AES_Encryption_Length_ERR");
        return (VS_ERROR);
    }

    if (1 != EVP_EncryptUpdate(&ctx, szCipherText, &inLen, szPlainText, inPlainTextLen)) {
        inDISP_LogPrintf("inCostco_TSP_AES_Encryption_Update_ERR");
        return (VS_ERROR);
    }
    inCipherTextLen = inLen;

    if (1 != EVP_EncryptFinal_ex(&ctx, szCipherText + inLen, &inLen)) {
        inDISP_LogPrintf("inCostco_TSP_AES_Encryption_Final_ERR");
        return (VS_ERROR);
    }
    inCipherTextLen += inLen;
    inDISP_LogPrintf("INFO(evp_encrypt): Output length: %d", inCipherTextLen);

    EVP_CIPHER_CTX_cleanup(&ctx);

    inDISP_LogPrintf("szCipherText--------------------------");
    vdCostco_TSP_PrintHexToString1(szCipherText, inCipherTextLen);

    inDISP_LogPrintf("inCostco_TSP_AES_Encryption_END");
    return (VS_SUCCESS);
}

void vdCostco_TSP_PrintHexToString1(unsigned char *szReadBuffer, int inBufferLen) {
    char szASCII[2048 + 1];
    int inTotalLEN = 0;
    int i = 0;
    int j = 0;

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("vdCostco_TSP_PrintHexToString1 Start");
    }
    inTotalLEN = inBufferLen;

    if (inTotalLEN > 0) {
        inDISP_LogPrintf("[ERM DATA	%04d                                                ]", inTotalLEN);
        inDISP_LogPrintf("[ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 ]");
        inDISP_LogPrintf("==============================================================================");

        memset(szASCII, 0x00, sizeof (szASCII));

        for (; i < inTotalLEN;) {
            //			SVC_HEX_2_DSP((const char *)&szReadBuffer[i ++], &szASCII[j], 1);
            inStoreBCDBin(&szASCII[j], &szASCII[j + 1], (int *) szReadBuffer[i++]);
            j += 2;
            szASCII[j++] = 0x20; /* 補空白 */

            if (i % 25 == 0) {
                inDISP_LogPrintf("[ %s]", szASCII);
                memset(szASCII, 0x00, sizeof (szASCII));
                j = 0;
            } else if (i == inTotalLEN) {
                inDISP_LogPrintf("[ %s]", szASCII);
                memset(szASCII, 0x00, sizeof (szASCII));
                j = 0;
            }
        } /* End for () .... */

        inDISP_LogPrintf("==============================================================================");
        /* 在這裡送結束 */
    }

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("vdCostco_TSP_PrintHexToString1 End");
    }
}

void vdCostco_TSP_PrintHexToString(unsigned char *szReadBuffer) {
    char szASCII[2048 + 1];
    int inTotalLEN = 0;
    int i = 0;
    int j = 0;

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("vdCostco_TSP_PrintHexToString Start");
    }
    inTotalLEN = strlen((const char *) szReadBuffer);

    if (inTotalLEN > 0) {
        inDISP_LogPrintf("[ERM DATA	%04d                                                ]", inTotalLEN);
        inDISP_LogPrintf("[ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 ]");
        inDISP_LogPrintf("==============================================================================");

        memset(szASCII, 0x00, sizeof (szASCII));

        for (; i < inTotalLEN;) {
            //			SVC_HEX_2_DSP((const char *)&szReadBuffer[i ++], &szASCII[j], 1);
            inStoreBCDBin(&szASCII[j], &szASCII[j + 1], (int *) szReadBuffer[i++]);
            j += 2;
            szASCII[j++] = 0x20; /* 補空白 */

            if (i % 25 == 0) {
                inDISP_LogPrintf("[ %s]", szASCII);
                memset(szASCII, 0x00, sizeof (szASCII));
                j = 0;
            } else if (i == inTotalLEN) {
                inDISP_LogPrintf("[ %s]", szASCII);
                memset(szASCII, 0x00, sizeof (szASCII));
                j = 0;
            }
        } /* End for () .... */

        inDISP_LogPrintf("==============================================================================");
        /* 在這裡送結束 */
    }

    if (ginDebug == VS_TRUE) {
        inDISP_LogPrintf("vdCostco_TSP_PrintHexToString End");
    }
}

int inCostco_TSP_Generate_TransactionKey(TRANSACTION_OBJECT *pobTran, int inTranMode) {
    char szISOSendData[ATS_ISO_SEND + 1];
    char szISOReceData[ATS_ISO_RECE + 1];
    int inTransKeyLen = 0;

    unsigned char szTransactionKey[32 + 1];
    unsigned char szPlaintext[30 + 1];
    unsigned char szTMK[32 + 1];
    unsigned char uszSendLRC;
    unsigned char uszReceLRC;
    unsigned char uszCRC;

    inDISP_LogPrintf("inCostco_TSP_Generate_TransactionKey Start");
    memset(szPlaintext, 0x00, sizeof (szPlaintext));

    if (inTranMode == D_HostValue) {
        szPlaintext[0] = pobTran->szTransCRC;
        memcpy((char *) &szPlaintext[1], pobTran->szRandom, pobTran->inRandomLen);
    } else {
        memcpy(&pobTran->szRandom, "\x43\x17\x7e\x3b\xd1\xce\x46\xdb\x4a\x82\xf1\xe2\x25\x24\xa9\x1b\x70\x7b\x91\xe1\x3b\x8a\x4d\x09\x9b\x89\x9f\x13\xe4", 29);
        memset(szISOSendData, 0x00, sizeof (szISOSendData));
        memset(szISOReceData, 0x00, sizeof (szISOReceData));

        memcpy(szISOSendData, &pobTran->szTransSendData[0], pobTran->inTransSendLen);
        uszSendLRC = (unsigned char) CAL_LRC(&szISOSendData[0]);

        if (pobTran->srBRec.uszOfflineBit != VS_TRUE) {
            memcpy(szISOReceData, &pobTran->szTransReceData[0], pobTran->inTransReceLen);
            inDISP_LogPrintf("Rece TPDU(%02x)(%02x)(%02x)(%02x)(%02x)",
                    szISOReceData[0], szISOReceData[1], szISOReceData[2], szISOReceData[3], szISOReceData[4]);
            uszReceLRC = (unsigned char) (CAL_LRC(&szISOReceData[0]));
        }

        /* ISO Print Debug */
        if (ginISODebug == VS_TRUE) {
            vdFUBON_ISO_ISOFormatDebug_PRINT((unsigned char *) &szISOSendData[0], pobTran->inTransSendLen);
            vdFUBON_ISO_ISOFormatDebug_PRINT((unsigned char *) &szISOReceData[0], pobTran->inTransReceLen);
        }

        if (pobTran->srBRec.uszOfflineBit != VS_TRUE)
            uszCRC = uszSendLRC ^ uszReceLRC;
        else
            uszCRC = uszSendLRC;

        pobTran->szTransCRC = uszCRC;

        inDISP_LogPrintf("ISO Send Data CRC(%02x)", uszSendLRC);
        inDISP_LogPrintf("ISO Rece Data CRC(%02x)", uszReceLRC);
        inDISP_LogPrintf("Xor CRC	      (%02x)(%02x)", uszCRC, pobTran->szTransCRC);

        inDISP_LogPrintf("inCostco_TSP_Generate_TransactionKey(%d)(%d)(%d)(%02x)", strlen(szISOSendData), strlen(szISOReceData), strlen(pobTran->szRandom), pobTran->szTransCRC);

        szPlaintext[0] = pobTran->szTransCRC;
        sprintf((char *) &szPlaintext[1], "%s", pobTran->szRandom);
    }

    /* Terminal Master Key + XOR Check sum of ciphered data fields | 29 randoms bytes*/
    memset(szTMK, 0x00, sizeof (szTMK));
    memset(pobTran->szTransKey, 0x00, sizeof (pobTran->szTransKey));

    inDISP_LogPrintf("szTerminalMasterKey--------------------");
    memcpy(&szTMK[0], &pobTran->szTerminalMasterKey[0], 32);
    vdCostco_TSP_PrintHexToString1(szTMK, 32);

    inTransKeyLen = inCostco_TSP_AES_DataEncrypt(szTMK, szPlaintext, 30, (unsigned char *) pobTran->szTransKey);

    if (1) {
        inDISP_LogPrintf("Transaction Key =");
        vdCostco_TSP_PrintHexToString((unsigned char *) pobTran->szTransKey);
    }

    inDISP_LogPrintf("inCostco_TSP_Generate_TransactionKey()_END(%d)", inTransKeyLen);
    return (VS_SUCCESS);
}

int CAL_LRC(char *Data) {
    int i = 0;
    int inLRC = 0; /* LRC的值 */

    /* 計算收到DATA的LRC （Stx Not Include）*/
    for (i = 0; i < strlen(Data); i++) {
        inLRC ^= Data[i];
    }

    return inLRC;
}

int inCostco_TSP_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket) {
    int inRetVal;
    int inSendTimeout = 3;
    int inReceiveTimeout = 3;
    int inReceiveSize = _COMM_RECEIVE_MAX_LENGTH_; /* 配合Ingenico */
    char szTimeOut[2 + 1] = {0};

    inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
    inDISP_PutGraphic(_SEND_, 0, _COORDINATE_Y_LINE_8_7_);

    inDISP_LogPrintf("-----[%s][%s][%d] START -----", __FILE__, __FUNCTION__, __LINE__);

    /* 如果沒設定TimeOut，就用EDC.dat內的TimeOut */
    if (inSendTimeout <= 0) {
        memset(szTimeOut, 0x00, sizeof (szTimeOut));
        inGetIPSendTimeout(szTimeOut);
        inSendTimeout = atoi(szTimeOut);
    }
    
    if ((inRetVal = inCOMM_Send(uszSendPacket, inSendLen, inSendTimeout, VS_TRUE)) != VS_SUCCESS) {
        inDISP_LogPrintf("inCOMM_Send() ERROR");
        return (VS_ERROR);
    }

    /* 這裡是用CPT.DAT的東西，所以一定要用 */
    /* 目前富邦規定是3秒，所以使用定義時的值 
     *如需要使用再打開  20181225 [SAM]*/
    //	memset(szHostResponseTimeOut, 0x00, sizeof(szHostResponseTimeOut));
    //	inRetVal = inGetHostResponseTimeOut(szHostResponseTimeOut);
    //
    //	if (inRetVal != VS_SUCCESS)
    //		return (VS_ERROR);
    //
    //	inReceiveTimeout = atoi(szHostResponseTimeOut);

    inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
    inDISP_PutGraphic(_RECEIVE_, 0, _COORDINATE_Y_LINE_8_7_);

    if ((inRetVal = inCOMM_Receive(uszRecvPacket, inReceiveSize, inReceiveTimeout, VS_TRUE)) != VS_SUCCESS) {
        inDISP_LogPrintf("inCOMM_Receive() ERROR");
        return (VS_ERROR);
    }

    return (VS_SUCCESS);
}

int inCostco_TSP_UnPackISO(TRANSACTION_OBJECT *pobTran, COSTCOPAY_OBJECT *srTMS, unsigned char *uszUnPackBuf, int inReceLen)
{
    char szASCII[8192 + 1];
    BYTE btTagData[4096 + 1];
    char szTraceLog[4096 + 1];
    char szObjectData[4096 + 1];
    char szTemplate[100 + 1];
    char szTag[8 + 1]; /* 宣告8byte長度的Tag */
    char szDispMsg[DISP_STR_LEN + 1];
    char szStatusCode[4 + 1];
    char szPrintBuffer[42 + 1];
    int inCnt = 0;
    int inDataSizes = 0;
    int i, inTagCnt, inLengthCnt;
    int inTemplate;
    int inLength;
    int inStatusCode = 0;
    int inRetVal = VS_ERROR;
        
    inDISP_LogPrintf("inCostco_TSP_UnPackISO()_START");
    
    if (memcmp(&uszUnPackBuf[0], "\x55", 1))
    {
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_ChineseFont("下載失敗", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_Wait(100);
        inDISP_BEEP(1, 0);
        return (VS_ERROR);
    }
    else
    {
        /* 確認接收資料的完整性(CRC 含Header/Data Len/Data -1 CRC) */
        if (inCostco_TSP_ISO_CheckCRC(srTMS, &uszUnPackBuf[0], (inReceLen - 1)) != VS_SUCCESS) {
            memset(srTMS->szRespCode, 0x00, sizeof (srTMS->szRespCode));
            strcpy(srTMS->szRespCode, "S7");

            inDISP_LogPrintf("inCostco_TSP_ISO_CheckCRC()_CRC_ERROR(%s)", srTMS->szRespCode);
            return (VS_ERROR);
        }

        /* 分析回傳總長度 */
        inCnt = 1;

        inLength = 0;
        inLengthCnt = 0;

        if ((uszUnPackBuf[inCnt] & 0x80) == 0x80) /* bit 8 表示後續幾個byte來表示長度 */ 
        {
            i = 0;
            inLengthCnt = uszUnPackBuf[inCnt] & 0x7F; /* 表示後續要幾個byte來表示長度 */
            while (i < inLengthCnt)
            {
                inLength += uszUnPackBuf[inCnt + inLengthCnt - i] << (8 * i); /* 真正長度 */
                i++;
            }
        } 
        else /* bit 8 表示長度只有一個byte 01~7F */
            inLength = uszUnPackBuf[inCnt]; /* 真正的長度 */
    }

    inCnt = inCnt + 1 + inLengthCnt;
    inDataSizes = inLength;
    inDISP_LogPrintf("inCostco_TSP_UnPackISO(Len = %d)(Cnt = %d)", inDataSizes, inCnt);

    for (; inCnt < inDataSizes;)
    {
        /* 解析Tag */
        inTagCnt = 0;
        while (VS_TRUE)
        {
            inTagCnt++; /* 表示此Tag的長度 */
            if (inTagCnt == 1) /* 檢查第一個byte */ 
            {
                if ((uszUnPackBuf[inCnt] & 0x1F) == 0x1F) /* bit 1~5 表示下一個byte也是Tag */
                    inCnt++;
                else 
                {
                    inCnt++;
                    break;
                }
            }
            else /* 檢查第二個以後的byte */ 
            {
                if ((uszUnPackBuf[inCnt] & 0x80) == 0x80) /* bit 8 表示下一個byte也是Tag */
                    inCnt++;
                else 
                {
                    inCnt++;
                    break;
                }
            }
        }

        memset(szTag, 0x00, sizeof (szTag));
        memcpy(szTag, &uszUnPackBuf[inCnt - inTagCnt], inTagCnt);

        /* 解析Length */
        inLength = 0;
        inLengthCnt = 0;
        if ((uszUnPackBuf[inCnt] & 0x80) == 0x80) /* bit 8 表示後續幾個byte來表示長度 */ 
        {
            i = 0;
            inLengthCnt = uszUnPackBuf[inCnt] & 0x7F; /* 表示後續要幾個byte來表示長度 */
            while (i < inLengthCnt)
            {
                inLength += uszUnPackBuf[inCnt + inLengthCnt - i] << (8 * i); /* 真正長度 */
                i++;
            }
        }
        else /* bit 8 表示長度只有一個byte 01~7F */
            inLength = uszUnPackBuf[inCnt]; /* 真正的長度 */

        inCnt = inCnt + 1 + inLengthCnt; /* 判斷長度的第一個byte沒有加一所以才加一 */

        /* 解析Value */

        /* Tag_20 Status */
        if (!memcmp(szTag, "\x20", 1)) 
        {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "  TAG_20(%02d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            /* Status code*/
            inStatusCode = (int) lnHexStringToLong(szASCII);
            inDISP_LogPrintf("EMS Status Code = [%ld]", inStatusCode);

            memset(srTMS->szRespCode, 0x00, sizeof (srTMS->szRespCode));
            switch (inStatusCode) {
                case 10000: memcpy(&srTMS->szRespCode[0], "00", 2);
                    break;
                case 10001: memcpy(&srTMS->szRespCode[0], "01", 2);
                    break;
                case 10002: memcpy(&srTMS->szRespCode[0], "02", 2);
                    break;
                case 10003: memcpy(&srTMS->szRespCode[0], "03", 2);
                    break;
                case 10004: memcpy(&srTMS->szRespCode[0], "04", 2);
                    break;
                case 10005: memcpy(&srTMS->szRespCode[0], "05", 2);
                    break;
                case 10006: memcpy(&srTMS->szRespCode[0], "06", 2);
                    break;
                case 10007: memcpy(&srTMS->szRespCode[0], "07", 2);
                    break;
                case 10008: memcpy(&srTMS->szRespCode[0], "08", 2);
                    break;
                case 10009: memcpy(&srTMS->szRespCode[0], "09", 2);
                    break;
                case 10010: memcpy(&srTMS->szRespCode[0], "10", 2);
                    break;
                case 10011: memcpy(&srTMS->szRespCode[0], "11", 2);
                    break;
                case 10012: memcpy(&srTMS->szRespCode[0], "12", 2);
                    break;
                case 10013: memcpy(&srTMS->szRespCode[0], "13", 2);
                    break;
                default: memcpy(&srTMS->szRespCode[0], "XX", 2);
                    inDISP_LogPrintf("inCostco_TSP_UnPackISO()_ERROR_Unknown(%s)", srTMS->szRespCode);
                    break;
            }

            if (memcmp(&srTMS->szRespCode[0], "00", 2))
            {
                    if (srTMS->inCode == ACTION_SATUS_REPORT)
                            return (VS_SUCCESS);
                    else
                            return (VS_ERROR);
            }

            continue;
        }

        /* Tag_09 Checksum */
        if (!memcmp(szTag, "\x09", 1)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "  TAG_09(%02d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            /* Checksum */
            pobTran->szTransCRC = btTagData[0];
            continue;
        }

        /* Tag_1A Random Bytes */
        if (!memcmp(szTag, "\x0A", 1)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "  TAG_0A(%02d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            /* Random Bytes */
            pobTran->inRandomLen = inLength;
            memset(pobTran->szRandom, 0x00, sizeof (pobTran->szRandom));
            memcpy(pobTran->szRandom, btTagData, inLength);
            continue;
        }

        /* Tag_bf01 File Binary Update Object */
        if (!memcmp(szTag, "\xbf\x01", 2)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "  TAG_bf01(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inCostco_TSP_ProcessObject_TLV(pobTran, btTagData, inLength);
            continue;
        }

        /* Tag_bf02 File Param Update Object */
        if (!memcmp(szTag, "\xbf\x02", 2)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "  TAG_bf02(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inCostco_TSP_ProcessObject_TLV(pobTran, btTagData, inLength);
            continue;
        }

        /* Tag_bf0b File Blacklist Update Object */
        if (!memcmp(szTag, "\xbf\x0b", 2)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "  TAG_bf0b(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inCostco_TSP_ProcessObject_TLV(pobTran, btTagData, inLength);
            continue;
        }

        /* Tag_bf10 Detoken Object VO */
        if (!memcmp(szTag, "\xbf\x10", 2)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "  TAG_bf0b(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inCostco_TSP_ProcessObject_TLV(pobTran, btTagData, inLength);
            continue;
        }

        /* Tag_B0 File */
        if (!memcmp(szTag, "\xb0", 1)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            /* 暫時錯誤太多，晚點處理 */
            //			inRetVal = inCB_ERM_ISO_Store(srTMS, (char *)&btTagData[0], inLength);

            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "  TAG_b0(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            /* 檔案合併處理 */
            continue;
        }

        inCnt += inLength; /* 當遇到沒有去解析的Tag也要移動位置 */
    } /* End for () .... */

    /* 暫時註解掉 */
    //	if (1) /* Debug */
    //	{
    //		if (srTMS->inCode == ACTION_CHECK_UPDATE_STATUS)
    //		{
    //			vdVERIX_PRINT_ChineseFont(" -----------------------------------------", PRT_NORMAL, PRT_COLUMN_42);
    //			vdVERIX_PRINT_ChineseFont("程式改版", PRT_HEIGHT, PRT_COLUMN_42);
    //			vdVERIX_PRINT_ChineseFont(" -----------------------------------------", PRT_NORMAL, PRT_COLUMN_42);
    //			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "下載時間= %s", szGetBin_DownloadDateTime());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "更新時間= %s", szGetBin_InstallDateTime());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //		 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "預期長度= %s", szGetBin_ExpectedLength());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //		 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "版本名稱= %s", szGetBin_VersionName());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //		 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "版本時間= %s", szGetBin_VersionDate());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //		 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //
    //		 	if (inGetBin_StatusCode() == 10009)
    //		 		sprintf(szPrintBuffer, "詢問結果= 要下載(%d)", inGetBin_StatusCode());
    //		 	else
    //		 		sprintf(szPrintBuffer, "詢問結果= 無須下載(%d)", inGetBin_StatusCode());
    //
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //		 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	memcpy(&szTemplate[0], szGetBin_CheckSum(), 20);
    //		 	sprintf(szPrintBuffer, "檢核碼= %s", szTemplate);
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //			vdVERIX_PRINT_ChineseFont("", PRT_NORMAL, PRT_COLUMN_32);
    //			vdVERIX_PRINT_ChineseFont("", PRT_NORMAL, PRT_COLUMN_32);
    //
    //		 	vdVERIX_PRINT_ChineseFont("參數下載", PRT_HEIGHT, PRT_COLUMN_42);
    //		 	vdVERIX_PRINT_ChineseFont(" -----------------------------------------", PRT_NORMAL, PRT_COLUMN_42);
    //			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "下載時間= %s", szGetParam_DownloadDateTime());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "更新時間= %s", szGetParam_InstallDateTime());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //		 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "預期長度= %s", szGetParam_ExpectedLength());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
/*
    //
    //		 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "版本名稱= %s", szGetParam_VersionName());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //			if (inGetParam_StatusCode() == 10009)
    //		 		sprintf(szPrintBuffer, "詢問結果= 要下載(%d)", inGetParam_StatusCode());
    //		 	else
    //		 		sprintf(szPrintBuffer, "詢問結果= 無須下載(%d)", inGetParam_StatusCode());
    //
    //			vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
*/
    //		 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	memcpy(&szTemplate[0], szGetParam_CheckSum(), 20);
    //		 	sprintf(szPrintBuffer, "檢核碼= %s", szTemplate);
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //			vdVERIX_PRINT_ChineseFont("", PRT_NORMAL, PRT_COLUMN_32);
    //			vdVERIX_PRINT_ChineseFont("", PRT_NORMAL, PRT_COLUMN_32);
    //			vdVERIX_PRINT_ChineseFont("黑名單下載", PRT_HEIGHT, PRT_COLUMN_42);
    //			vdVERIX_PRINT_ChineseFont(" -----------------------------------------", PRT_NORMAL, PRT_COLUMN_42);
    //			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "下載時間= %s", szGetBList_DownloadDateTime());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "更新時間= %s", szGetBList_InstallDateTime());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //		 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "預期長度= %s", szGetBList_ExpectedLength());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //		 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	sprintf(szPrintBuffer, "版本名稱= %s", szGetBList_RevisionFilename());
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //			if (inGetBList_StatusCode() == 10009)
    //		 		sprintf(szPrintBuffer, "詢問結果= 要下載(%d)", inGetBList_StatusCode());
    //		 	else
    //		 		sprintf(szPrintBuffer, "詢問結果= 無須下載(%d)", inGetBList_StatusCode());
    //
    //			vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //
    //		 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
    //		 	memcpy(&szTemplate[0], szGetBList_CheckSum(), 20);
    //		 	sprintf(szPrintBuffer, "檢核碼= %s", szTemplate);
    //		 	vdVERIX_PRINT_ChineseFont(szPrintBuffer, PRT_NORMAL, PRT_COLUMN_32);
    //		 	vdVERIX_PRINT_ChineseFont(" -----------------------------------------", PRT_NORMAL, PRT_COLUMN_42);
    //		 	vdVERIX_PRINT_ChineseFont("", PRT_NORMAL, PRT_COLUMN_42);
    //		 	vdVERIX_PRINT_ChineseFont("", PRT_NORMAL, PRT_COLUMN_42);
    //		 }
    //	}

    inDISP_LogPrintf("inCB_ERM_ISO_UnPack_END");
    return (VS_SUCCESS);
}

int inCostco_TSP_ISO_CheckCRC(COSTCOPAY_OBJECT *srTMS, unsigned char *szCheckData, int inSizes) {
    int i;
    char szCRC[1 + 1];

    inDISP_LogPrintf("inCostco_TSP_ISO_CheckCRC()_START");

    memset(szCRC, 0x00, sizeof (szCRC));

    for (i = 0; i < inSizes; i++) {
        szCRC[0] ^= szCheckData[i];
    }

    inDISP_LogPrintf("Data Start [%02x]", szCheckData[0]);
    inDISP_LogPrintf("Data End   [%02x]", szCheckData[i]);

    if (szCRC[0] != szCheckData[inSizes]) {
        inDISP_LogPrintf("CRC [%d] : [%02x] / [%02x]", inSizes, szCRC[0], szCheckData[inSizes]);
        inDISP_LogPrintf("inCostco_TSP_ISO_CheckCRC()_ERROR");
        return (VS_ERROR);
    } else {
        inDISP_LogPrintf("inCostco_TSP_ISO_CheckCRC()_SUCCESS");
        return (VS_SUCCESS);
    }
}

long lnHexStringToLong(char *szHex) {
    long lnRetVal;
    int x;

    x = 0;
    lnRetVal = 0L;

    /** / pdebug(( "Hex %s", szHex  )); / **/

    while (szHex[x]) {
        lnRetVal = lnRetVal * 16 + (isdigit(szHex[x]) ? (szHex[x] - '0') :
                (szHex[x] - 'A' + 10));

        x++;
        /** / pdebug(("lnRetVal %li %i", lnRetVal, x)); / **/
    }
    return ( lnRetVal);

}

int inCostco_TSP_ProcessObject_TLV(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf, int inDataSizes) {
    char szASCII[8192 + 1];
    BYTE btTagData[4096 + 1];
    char szTraceLog[1024 + 1];
    char szTemplate[100 + 1];
    char szTag[8 + 1]; /* 宣告8byte長度的Tag */
    char szDispMsg[DISP_STR_LEN + 1];
    char szStatusCode[4 + 1];
    char szPrintBuffer[42 + 1];
    int inCnt = 0;
    int i, inTagCnt, inLengthCnt;
    int inTemplate;
    int inLength = 0;
    int inLen = 0;
    int inStatusCode = 0;

    inDISP_LogPrintf("inCostco_TSP_ProcessObject_TLV()_START(%d)", inDataSizes);
    
    /* 520抄過來，V3目前用HDT，先保留 */
    //	if (inLoadERMRec(0) < 0)
    //	{
    //		inDISP_LogPrintf("inCostco_TSP_ProcessObject_TLV()_inLoadERMRec()_ERROR");
    //		return (VS_ERROR); /* 下載清單【TMSF.txt】 */
    //	}

    for (; inCnt < inDataSizes;)
    {
        /* 解析Tag */
        inTagCnt = 0;
        while (VS_TRUE)
        {
            inTagCnt++; /* 表示此Tag的長度 */
            if (inTagCnt == 1) /* 檢查第一個byte */
            {
                if ((uszUnPackBuf[inCnt] & 0x1F) == 0x1F) /* bit 1~5 表示下一個byte也是Tag */
                    inCnt++;
                else 
                {
                    inCnt++;
                    break;
                }
            }
            else /* 檢查第二個以後的byte */ 
            {
                if ((uszUnPackBuf[inCnt] & 0x80) == 0x80) /* bit 8 表示下一個byte也是Tag */
                    inCnt++;
                else
                {
                    inCnt++;
                    break;
                }
            }
        }

        memset(szTag, 0x00, sizeof (szTag));
        memcpy(szTag, &uszUnPackBuf[inCnt - inTagCnt], inTagCnt);


        /* 解析Length */
        inLength = 0;
        inLengthCnt = 0;
        if ((uszUnPackBuf[inCnt] & 0x80) == 0x80) /* bit 8 表示後續幾個byte來表示長度 */ {
            i = 0;
            inLengthCnt = uszUnPackBuf[inCnt] & 0x7F; /* 表示後續要幾個byte來表示長度 */
            while (i < inLengthCnt) {
                inLength += uszUnPackBuf[inCnt + inLengthCnt - i] << (8 * i); /* 真正長度 */
                i++;
            }
        } else /* bit 8 表示長度只有一個byte 01~7F */
            inLength = uszUnPackBuf[inCnt]; /* 真正的長度 */

        inCnt = inCnt + 1 + inLengthCnt; /* 判斷長度的第一個byte沒有加一所以才加一 */

        /* 解析Value */
        /*  -------- File Binary Update Object -------- */

        /* Tag_FFBF8002 TSP Transaction ID */
        if (!memcmp(szTag, "\xFF\xBF\x80\x02", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8002(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            /* TSP Transaction ID */
            memcpy(pobTran->srBRec.szTSPTransactionID, btTagData, inLength);
            continue;
        }

        /* Tag_FFBF8003 PAN */
        if (!memcmp(szTag, "\xFF\xBF\x80\x03", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8003(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            /* PAN */
            pobTran->inEncryptPanLen = inLength;
            memcpy(pobTran->szEncryptPan, btTagData, inLength);
            continue;
        }

        /* Tag_FFBF8004 Expiry Date */
        if (!memcmp(szTag, "\xFF\xBF\x80\x04", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8004(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            /* Expiry Date */
            pobTran->inEncryptExpireDateLen = inLength;
            memcpy(pobTran->szEncryptExpireDate, btTagData, inLength);
            continue;
        }

        /* Tag_FFBF8101 Check Update Status */
        if (!memcmp(szTag, "\xFF\xBF\x81\x01", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8101(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            /* Status code*/
            inStatusCode = (int) lnHexStringToLong(szASCII);
            inDISP_LogPrintf("-Object Status Code_Binary = [%ld]", inStatusCode);
            /* 暫時不管，先註解掉 */
//            vdSetBin_StatusCode(inStatusCode);

            switch (inStatusCode) {
                case 10000: memcpy(&pobTran->srBRec.szRespCode[0], "00", 2);
                    break;
                case 10001: memcpy(&pobTran->srBRec.szRespCode[0], "01", 2);
                    break;
                case 10002: memcpy(&pobTran->srBRec.szRespCode[0], "02", 2);
                    break;
                case 10003: memcpy(&pobTran->srBRec.szRespCode[0], "03", 2);
                    break;
                case 10004: memcpy(&pobTran->srBRec.szRespCode[0], "04", 2);
                    break;
                case 10005: memcpy(&pobTran->srBRec.szRespCode[0], "05", 2);
                    break;
                case 10006: memcpy(&pobTran->srBRec.szRespCode[0], "06", 2);
                    break;
                case 10007: memcpy(&pobTran->srBRec.szRespCode[0], "07", 2);
                    break;
                case 10008: memcpy(&pobTran->srBRec.szRespCode[0], "08", 2);
                    break;
                case 10009: memcpy(&pobTran->srBRec.szRespCode[0], "09", 2);
                    break;
                case 10010: memcpy(&pobTran->srBRec.szRespCode[0], "10", 2);
                    break;
                case 10011: memcpy(&pobTran->srBRec.szRespCode[0], "11", 2);
                    break;
                case 10012: memcpy(&pobTran->srBRec.szRespCode[0], "12", 2);
                    break;
                case 10013: memcpy(&pobTran->srBRec.szRespCode[0], "13", 2);
                    break;
                default: memcpy(&pobTran->srBRec.szRespCode[0], "XX", 2);
                    inDISP_LogPrintf("inCostco_TSP_UnPackISO()_ERROR_Unknown(%s)", pobTran->srBRec.szRespCode);
                    break;
            }

            continue;
        }

        /* Tag_FFBF8102 MD5 */
        if (!memcmp(szTag, "\xFF\xBF\x81\x02", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8102(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            continue;
        }

        /* Tag_FFBF8103 SHA1 */
        if (!memcmp(szTag, "\xFF\xBF\x81\x03", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            
            /* 暫時註解 */
//            vdSetBin_CheckSum(szASCII);
            
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8103(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            continue;
        }

        /* Tag_FFBF8104 SHA256 */
        if (!memcmp(szTag, "\xFF\xBF\x81\x04", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8104(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            continue;
        }

        /* Tag_FFBF8105 Scheduled Download Date Time */
        if (!memcmp(szTag, "\xFF\xBF\x81\x05", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8105(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Scheduled Download Date Time = (%s)", btTagData);
            
            /* 暫時註解 */
//            vdSetBin_DownloadDateTime((const char *) btTagData);
            continue;
        }

        /* Tag_FFBF8106 Scheduled Install Date Time */
        if (!memcmp(szTag, "\xFF\xBF\x81\x06", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8106(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Scheduled Install Date Time = (%s)", btTagData);
            /* 暫時註解 */
//            vdSetBin_InstallDateTime((const char *) btTagData);
            continue;
        }

        /* Tag_FFBF8107 Expected Length */
        if (!memcmp(szTag, "\xFF\xBF\x81\x07", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8107(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            inLen = 0;
            inLen = (int) lnHexStringToLong(szASCII);
            inDISP_LogPrintf("-Expected Length = (%d)", inLen);

            memset(szTemplate, 0x00, sizeof (szTemplate));
            sprintf(&szTemplate[0], "%02d", inLen);
            /* 暫時註解 */
//            vdSetBin_ExpectedLength(szTemplate);
            continue;
        }

        /* Tag_FFBF810A Version Name */
        if (!memcmp(szTag, "\xFF\xBF\x81\x0A", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF810A(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Version Name = (%s)", btTagData);
            /* 暫時註解 */
//            vdSetBin_VersionName((const char *) btTagData);
            continue;
        }

        /* Tag_FFBF810B Terminal Vendor */
        if (!memcmp(szTag, "\xFF\xBF\x81\x0B", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF810B(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Terminal Vendor = (%s)", btTagData);
            continue;
        }

        /* Tag_FFBF810C Terminal Type */
        if (!memcmp(szTag, "\xFF\xBF\x81\x0C", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF810C(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Terminal Type = (%s)", btTagData);
            continue;
        }

        /* Tag_FFBF810D Terminal Version Type */
        if (!memcmp(szTag, "\xFF\xBF\x81\x0D", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF810D(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Terminal Version Type = (%s)", btTagData);
            continue;
        }

        /* Tag_FFBF810E Version Date */
        if (!memcmp(szTag, "\xFF\xBF\x81\x0E", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF810E(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Version Date = (%s)", btTagData);
            /* 暫時註解 */
//            vdSetBin_VersionDate((const char *) btTagData);
            continue;
        }

        /* -------- File Param Update Object -------- */

        /* Tag_FFBF8201 Check Update Status */
        if (!memcmp(szTag, "\xFF\xBF\x82\x01", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8201(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            /* Status code*/
            inStatusCode = (int) lnHexStringToLong(szASCII);
            inDISP_LogPrintf("-Object Status Code_Param = [%ld]", inStatusCode);
            /* 暫時註解 */
//            vdSetParam_StatusCode(inStatusCode);

            switch (inStatusCode) {
                case 10000: memcpy(&pobTran->srBRec.szRespCode[0], "00", 2);
                    break;
                case 10001: memcpy(&pobTran->srBRec.szRespCode[0], "01", 2);
                    break;
                case 10002: memcpy(&pobTran->srBRec.szRespCode[0], "02", 2);
                    break;
                case 10003: memcpy(&pobTran->srBRec.szRespCode[0], "03", 2);
                    break;
                case 10004: memcpy(&pobTran->srBRec.szRespCode[0], "04", 2);
                    break;
                case 10005: memcpy(&pobTran->srBRec.szRespCode[0], "05", 2);
                    break;
                case 10006: memcpy(&pobTran->srBRec.szRespCode[0], "06", 2);
                    break;
                case 10007: memcpy(&pobTran->srBRec.szRespCode[0], "07", 2);
                    break;
                case 10008: memcpy(&pobTran->srBRec.szRespCode[0], "08", 2);
                    break;
                case 10009: memcpy(&pobTran->srBRec.szRespCode[0], "09", 2);
                    break;
                case 10010: memcpy(&pobTran->srBRec.szRespCode[0], "10", 2);
                    break;
                case 10011: memcpy(&pobTran->srBRec.szRespCode[0], "11", 2);
                    break;
                case 10012: memcpy(&pobTran->srBRec.szRespCode[0], "12", 2);
                    break;
                case 10013: memcpy(&pobTran->srBRec.szRespCode[0], "13", 2);
                    break;
                default: memcpy(&pobTran->srBRec.szRespCode[0], "XX", 2);
                    inDISP_LogPrintf("inCB_ERM_ISO_UnPack()_ERROR_Unknown(%s)", pobTran->srBRec.szRespCode);
                    break;
            }

            continue;
        }

        /* Tag_FFBF8202 MD5 */
        if (!memcmp(szTag, "\xFF\xBF\x82\x02", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8202(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            continue;
        }

        /* Tag_FFBF8203 SHA1 */
        if (!memcmp(szTag, "\xFF\xBF\x82\x03", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            /* 暫時註解 */
//            vdSetParam_CheckSum(szASCII);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8203(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            continue;
        }

        /* Tag_FFBF8204 SHA256 */
        if (!memcmp(szTag, "\xFF\xBF\x82\x04", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8204(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            continue;
        }

        /* Tag_FFBF8205 Scheduled Download Date Time */
        if (!memcmp(szTag, "\xFF\xBF\x82\x05", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8205(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Scheduled Download Date Time = (%s)", btTagData);
            /* 暫時註解 */
//            vdSetParam_DownloadDateTime((const char *) btTagData);
            continue;
        }

        /* Tag_FFBF8206 Scheduled Install Date Time */
        if (!memcmp(szTag, "\xFF\xBF\x82\x06", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8206(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Scheduled Install Date Time = (%s)", btTagData);
            /* 暫時註解 */
//            vdSetParam_InstallDateTime((const char *) btTagData);
            continue;
        }

        /* Tag_FFBF8207 Expected Length */
        if (!memcmp(szTag, "\xFF\xBF\x82\x07", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8207(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            inLen = 0;
            inLen = (int) lnHexStringToLong(szASCII);
            inDISP_LogPrintf("-Expected Length = (%d)", inLen);

            memset(szTemplate, 0x00, sizeof (szTemplate));
            sprintf(&szTemplate[0], "%02d", inLen);
            /* 暫時註解 */
//            vdSetParam_ExpectedLength(szTemplate);
            continue;
        }

        /* Tag_FFBF8209 Version Name */
        if (!memcmp(szTag, "\xFF\xBF\x82\x09", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8209(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Version Name = (%s)", btTagData);
            /* 暫時註解 */
//            vdSetParam_VersionName((const char *) btTagData);
            continue;
        }

        /* -------- File Blacklist Update Object -------- */

        /* Tag_FFBF8B01 Check Update Status */
        if (!memcmp(szTag, "\xFF\xBF\x8B\x01", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8B01(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            /* Status code*/
            inStatusCode = (int) lnHexStringToLong(szASCII);
            inDISP_LogPrintf("-Object Check Status Code_Black = [%ld]", inStatusCode);
            /* 暫時註解 */
//            vdSetBList_StatusCode(inStatusCode);

            switch (inStatusCode) {
                case 10000: memcpy(&pobTran->srBRec.szRespCode[0], "00", 2);
                    break;
                case 10001: memcpy(&pobTran->srBRec.szRespCode[0], "01", 2);
                    break;
                case 10002: memcpy(&pobTran->srBRec.szRespCode[0], "02", 2);
                    break;
                case 10003: memcpy(&pobTran->srBRec.szRespCode[0], "03", 2);
                    break;
                case 10004: memcpy(&pobTran->srBRec.szRespCode[0], "04", 2);
                    break;
                case 10005: memcpy(&pobTran->srBRec.szRespCode[0], "05", 2);
                    break;
                case 10006: memcpy(&pobTran->srBRec.szRespCode[0], "06", 2);
                    break;
                case 10007: memcpy(&pobTran->srBRec.szRespCode[0], "07", 2);
                    break;
                case 10008: memcpy(&pobTran->srBRec.szRespCode[0], "08", 2);
                    break;
                case 10009: memcpy(&pobTran->srBRec.szRespCode[0], "09", 2);
                    break;
                case 10010: memcpy(&pobTran->srBRec.szRespCode[0], "10", 2);
                    break;
                case 10011: memcpy(&pobTran->srBRec.szRespCode[0], "11", 2);
                    break;
                case 10012: memcpy(&pobTran->srBRec.szRespCode[0], "12", 2);
                    break;
                case 10013: memcpy(&pobTran->srBRec.szRespCode[0], "13", 2);
                    break;
                default: memcpy(&pobTran->srBRec.szRespCode[0], "XX", 2);
                    inDISP_LogPrintf("inCB_ERM_ISO_UnPack()_ERROR_Unknown(%s)", pobTran->srBRec.szRespCode);
                    break;
            }

            continue;
        }

        /* Tag_FFBF8B02 MD5 */
        if (!memcmp(szTag, "\xFF\xBF\x8B\x02", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8B02(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            continue;
        }

        /* Tag_FFBF8B03 SHA1 */
        if (!memcmp(szTag, "\xFF\xBF\x8B\x03", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8B03(%04d)(%s)", inLength, szASCII);
            /* 暫時註解 */
//            vdSetBList_CheckSum(szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            continue;
        }

        /* Tag_FFBF8B04 SHA256 */
        if (!memcmp(szTag, "\xFF\xBF\x8B\x04", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8B04(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            continue;
        }

        /* Tag_FFBF8B05 Scheduled Download Date Time */
        if (!memcmp(szTag, "\xFF\xBF\x8B\x05", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8B05(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Scheduled Download Date Time = (%s)", btTagData);
            /* 暫時註解 */
//            vdSetBList_DownloadDateTime((const char *) btTagData);
            continue;
        }

        /* Tag_FFBF8B06 Scheduled Install Date Time */
        if (!memcmp(szTag, "\xFF\xBF\x8B\x06", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8B06(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Scheduled Install Date Time = (%s)", btTagData);
            /* 暫時註解 */
//            vdSetBList_InstallDateTime((const char *) btTagData);
            continue;
        }

        /* Tag_FFBF8B07 Expected Length */
        if (!memcmp(szTag, "\xFF\xBF\x8B\x07", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8B07(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);

            inLen = 0;
            inLen = (int) lnHexStringToLong(szASCII);
            inDISP_LogPrintf("-Expected Length = (%d)", inLen);

            memset(szTemplate, 0x00, sizeof (szTemplate));
            sprintf(&szTemplate[0], "%02d", inLen);
            /* 暫時註解 */
//            vdSetBList_ExpectedLength(szTemplate);
            continue;
        }

        /* Tag_FFBF8B09 Blacklist Revision Filename */
        if (!memcmp(szTag, "\xFF\xBF\x8B\x09", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8B09(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Blacklist Revision Filename = (%s)", btTagData);
            /* 暫時註解 */
//            vdSetBList_RevisionFilename((const char *) btTagData);
            continue;
        }

        /* Tag_FFBF8B0A Blacklist Type */
        if (!memcmp(szTag, "\xFF\xBF\x8B\x0A", 4)) {
            memset(btTagData, 0x00, sizeof (btTagData));
            memcpy((char *) &btTagData[0], &uszUnPackBuf[inCnt], inLength);
            inCnt += inLength;

            /* Print debug */
            memset(szASCII, 0x00, sizeof (szASCII));
            inFunc_BCD_to_ASCII(&szASCII[0], &btTagData[0], inLength);
            memset(szTraceLog, 0x00, sizeof (szTraceLog));
            sprintf(szTraceLog, "-Object TAG_FFBF8B0A(%04d)(%s)", inLength, szASCII);
            inDISP_LogPrintf("%s", szTraceLog);
            inDISP_LogPrintf("-Blacklist Type = (%s)", btTagData);
            continue;
        }

        inCnt += inLength; /* 當遇到沒有去解析的Tag也要移動位置 */
    } /* End for () .... */

//    if (inSaveERMRec(0) < 0) {
//        LOG_PRINTF(("inCostco_TSP_ProcessObject_TLV()_inLoadERMRec()_ERROR"));
//        return (VS_ERROR);
//    }

    inDISP_LogPrintf("inCostco_TSP_ProcessObject_TLV End");
    return (VS_SUCCESS);
}

int inCostco_TSP_AES_DataEncrypt(unsigned char *szKey, unsigned char *szPlainText, int inPlainTextLen ,unsigned char *szCipherText)
{
	EVP_CIPHER_CTX ctx;

	unsigned char	szOutputData[128 + 1];
	char	szTemplate[42 + 1], szPrintBuf[42 + 1];
	int	i ,inNumPad = 0;
	int	inLen = 0;
	int	inCipherTextLen = 0;
	int	inRetVal = -1;
	int	inCipherBlockSize, inCipherKeyLength, inCipherIvLength;
	int	inCipherLen = 0;

	unsigned char szIV[] =
	{ /* Only need 16 bytes... */
		0xa5, 0x83, 0xb4, 0x1b, 0x1b, 0x5a, 0x4b, 0xf2,
		0xc7, 0x0c, 0xf9, 0xdf, 0xed, 0xc6, 0x2c, 0xbc, 0
	};

	inDISP_LogPrintf("inCostco_TSP_AES_DataEncrypt Len = [%d]", inPlainTextLen);

	/* PKCS */
	inNumPad = 16 - (inPlainTextLen % 16); /* 計算需要Padding數量 */

	for (i = 0; i < inNumPad; i++)
		szPlainText[inPlainTextLen + i] = inNumPad;

	inPlainTextLen = inPlainTextLen + inNumPad;

	inDISP_LogPrintf("szPlainText--------------------------");
	vdCostco_TSP_PrintHexToString1(szPlainText, inPlainTextLen);


	EVP_CIPHER_CTX_init(&ctx);
	EVP_EncryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, (const unsigned char *)szKey, (const unsigned char *)szIV);

	/* 關閉padding 功能,預設開啟 */
	EVP_CIPHER_CTX_set_padding(&ctx, 0);

	inCipherBlockSize = EVP_CIPHER_CTX_block_size(&ctx);
	inCipherKeyLength = EVP_CIPHER_CTX_key_length(&ctx);
	inCipherIvLength  = EVP_CIPHER_CTX_iv_length(&ctx);

	inDISP_LogPrintf("INFO(evp_encrypt): Input length: %d", inPlainTextLen);
	inDISP_LogPrintf("INFO(evp_encrypt): block size: %d", inCipherBlockSize);
	inDISP_LogPrintf("INFO(evp_encrypt): key length: %d", inCipherKeyLength);
	inDISP_LogPrintf("INFO(evp_encrypt): IV length:  %d", inCipherIvLength);

  	if ((inCipherKeyLength > 32) || (inCipherIvLength  > 16))
	{
		inDISP_LogPrintf("Len Error");
		return (VS_ERROR);
	}

	if (1 != EVP_EncryptUpdate(&ctx, szCipherText, &inLen, szPlainText, inPlainTextLen))
	{
		inDISP_LogPrintf("Update Error");
		return (VS_ERROR);
	}
	inCipherTextLen = inLen;

	if (1 != EVP_EncryptFinal_ex(&ctx, szCipherText + inLen, &inLen))
	{
		inDISP_LogPrintf("EncryptFinal Final ERR");
		return (VS_ERROR);
	}
	inCipherTextLen += inLen;
	inDISP_LogPrintf("INFO(evp_encrypt): Output length: %d", inCipherTextLen);

	EVP_CIPHER_CTX_cleanup(&ctx);

	inDISP_LogPrintf("szCipherText--------------------------");
	vdCostco_TSP_PrintHexToString1(szCipherText, inCipherTextLen);

	inDISP_LogPrintf("inCostco_TSP_AES_DataEncrypt End");
	return (inCipherTextLen);
}

int inCostco_TSP_ProcessScanPay(TRANSACTION_OBJECT *pobTran)
{
        long    lnTimeout = 0;
        char    szKey = -1;
        char    szCommmode[1 + 1] = {0};
        char    szDemoMode[2 + 1] = {0};
        char	szCTLSMode[1 + 1] = {0};
        char	szExpDateCheck[2 + 1] = {0};
        char	szExpireDate[4 + 1] = {0};
        char	szTRTFileName[16 + 1] = {0};
        int	inA30_Send = VS_FALSE;
        int     i = 0;
        int	inResult = VS_FALSE;
        
        inDISP_LogPrintf("inCostco_TSP_ProcessScanPay Start");
        
	memset(szCTLSMode, 0x00, sizeof(szCTLSMode));
	inGetContactlessReaderMode(szCTLSMode);
        
        inDISP_LogPrintf("CTLS Mode = %s", szCTLSMode);
        
        /* 防呆 */
        memset(szCommmode,0x00,sizeof(szCommmode));
        /* 取得通訊模式，目前沒有撥接擋掉 */
        if (inGetCommMode(szCommmode) == VS_ERROR)
        {
                inDISP_ClearAll();
                inDISP_ChineseFont("取得通訊模式失敗", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
                return (VS_ERROR);
        }
        if (!memcmp(szCommmode, _COMM_MODEM_MODE_, 1))
        {
		inDISP_ClearAll();
                inDISP_ChineseFont("掃碼付不支援撥接", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
                return (VS_ERROR);
        }
        if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
            !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
        {
            inDISP_LogPrintfWithFlag("-----[%s][%s][%d] Not External  END -----",__FILE__, __FUNCTION__, __LINE__);
            return VS_ERROR;
        }
        /* 防呆，如果外接設備已有動作，先中斷 */
        if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
		inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
        
        inDISP_Wait(2000);
        
        inDISP_ClearAll();
        inDISP_PutGraphic(_SCAN_PROCESS_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜掃碼交易＞ */
        inDISP_PutGraphic(_SCAN_CODE_, 0, _COORDINATE_Y_LINE_8_3_);         /* 請用外接設備掃描條碼 */
        
        /* [外接設備設定] */
        if(inMultiFunc_JsonCallSlave(pobTran, _MULTI_SCAN_TRANS_NO_, &gstMultiOb) != VS_SUCCESS)
        {
                inDISP_LogPrintf("inMultiFunc_JsonCallSlave Error 2");
                return VS_ERROR;
        }
        
        lnTimeout = 15;
	inDISP_TimeoutStart(lnTimeout);
        
        /* 先設定為讀卡失敗，有成功再設定回沒設定錯誤碼 */
	inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_READER_CARD_ERROR_);
        
        /* 進迴圈前先清MSR BUFFER */
	inCARD_Clean_MSR_Buffer();
	/* 啟動掃碼器 */
	while(1)
	{
                if(1)   /*  由V3發取消 Or Timeout指令 */  
                {
                        /* ------------偵測key in------------------ */
                        szKey = -1;
                        szKey = uszKBD_Key();

                        /* 感應倒數時間 && Display Countdown */
                        if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
                        {
                                /* 感應時間到Timeout */
                                inDISP_LogPrintf("TimeOut 1");
                                szKey = _KEY_TIMEOUT_;
                        }
                        if (szKey == _KEY_TIMEOUT_)
                        {
                                inDISP_LogPrintf("TimeOut 2");
                                if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                                    !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
                                {
                                        if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
                                                inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
                                } else
                                {
                                        /* 取消感應交易 */
                                        inCTLS_CancelTransacton_Flow();
                                }
                                /* Timeout */
                                inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_TIMEOUT_);
                                inResult = (VS_TIMEOUT);
                                break;
                        }
                        else if (szKey == _KEY_CANCEL_)
                        {
                                inDISP_LogPrintf("Cancel");
                                if (!memcmp(szCTLSMode, _EXTERNAL_DEVICE_A30_, strlen(_EXTERNAL_DEVICE_A30_)) ||
                                    !memcmp(szCTLSMode, _EXTERNAL_DEVICE_V3C_, strlen(_EXTERNAL_DEVICE_V3C_)) )
                                {
                                        if (gstMultiOb.stMulti_TransData.unzAlreadySend == VS_TRUE)
                                                inMultiFunc_Host_CallSendErrorFunc(pobTran, _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_);
                                } else
                                {
                                        /* 取消感應交易 */
                                        inCTLS_CancelTransacton_Flow();
                                }
                                /* Cancel */
                                inResult = (VS_USER_CANCEL);
                                break;
                        }
                }
                
                /* 感應倒數時間 && Display Countdown */
                if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
                {
                        /* 感應時間到Timeout */
                        inDISP_LogPrintf("Scan TimeOut");
                        inFUNC_SetEcrErrorMag(pobTran, _ECR_RESPONSE_CODE_TIMEOUT_);
                        inResult = VS_TIMEOUT;
                        break;
                }

                /* 偵測是否掃碼 */
                if (inA30_Send == VS_FALSE)
                {
                        for (i = 0; i < 3; i++)
                        {
                                inResult = inMultiFunc_Host_RecvCtlsData(pobTran);
                                if(inResult == VS_SUCCESS)
                                {
                                        inA30_Send = VS_TRUE;
                                        inResult = inMultiFunc_CallSlaveCtlsResult(pobTran);
                                        if (inResult == VS_SUCCESS)
                                        {
                                                inDISP_LogPrintf("Get Scan Data Success");
                                                inDISP_LogPrintf("Scan Data = [%s]", pobTran->szQrData);
                                                inDISP_LogPrintf("Scan Data Len = [%d]", pobTran->inQrDataLen);
                                                break;
                                        }
                                        else if(inResult == VS_USER_CANCEL)
                                        {
                                                inDISP_LogPrintf("A30 Scan Cancel");
                                                break;
                                        }
                                        else if(inResult == VS_ERROR)   /* A30 發TimeOut */
                                        {
                                                inDISP_LogPrintf("A30 Scan Error/TimeOut");
                                                break;
                                        }
                                }
                                inDISP_Wait(100);
                        }
                }
                else if (inA30_Send == VS_TRUE)
                {
                        break;
                }
        }

	if (inResult != VS_SUCCESS)
	{
		inDISP_ClearAll();

                if (inResult == VS_TIMEOUT)
                {
                        inDISP_ChineseFont("交易逾時", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
                }
                else if (inResult == VS_USER_CANCEL)
                {
                        inDISP_ChineseFont("交易取消", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
                }
                else
                {
                        inDISP_ChineseFont("感應流程錯誤", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
                }

		inDISP_BEEP(3,200);
		return (VS_ERROR);
	}
        
        memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
        
	if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
	{
		inResult = LoadCreditScanHost();
	}
	else if (!memcmp(szTRTFileName, _TRT_FILE_NAME_REDEMPTION_, strlen(_TRT_FILE_NAME_REDEMPTION_)))
	{
		inResult = LoadRedeemScanHost();
	}
	else if (!memcmp(szTRTFileName, _TRT_FILE_NAME_INSTALLMENT_, strlen(_TRT_FILE_NAME_INSTALLMENT_)))
	{
		inResult = LoadInstScanHost();
	}

	if (inResult != VS_SUCCESS)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_Msg_BMP(_ERR_FUNC_CLOSE_, _COORDINATE_Y_LINE_8_6_, _BEEP_1TIMES_MSG_, 2, "", 0);
		return (VS_ERROR);
	}

        memset(szDemoMode, 0x00, sizeof (szDemoMode));
	inGetDemoMode(szDemoMode);

        if (!memcmp(&szDemoMode[0], "Y", strlen("Y")))
        {
                memcpy(pobTran->srBRec.szExpDate, "2705", 4);
                memcpy(pobTran->srBRec.szPAN, "4284300179998110", 16);

                inDISP_LogPrintf("pobTran->srBRec.szPAN = [%s]", pobTran->srBRec.szPAN);
                inDISP_LogPrintf("pobTran->srBRec.szExpDate = [%s]", pobTran->srBRec.szExpDate);

                inResult = inCARD_GetBin(pobTran);
                if (inResult == VS_SUCCESS)
                {
                        /* 判斷有效期 */
                        memset(szExpDateCheck, 0x00, sizeof (szExpDateCheck));
                        inGetExpiredDateCheck(szExpDateCheck);
                        
                        if (!memcmp(&szExpDateCheck, "Y", 1))
                        {
                                if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
                                {
                                        inDISP_LogPrintf("CARD ValidateExpDate error");
                                        inResult = VS_ERROR;
                                }
                        }
                }
                else
                {
                        memcpy(&pobTran->srBRec.szExpDate[2], &szExpireDate[0], 2); /* MM */
                        inDISP_LogPrintf("inCARD_GetBin error = [%d]", inResult);
                }
        }
        else
        {
                inResult = inCostco_ScanPay_Online_Flow(pobTran);
        }
	pobTran->fSendToCostcoPayHost = VS_FALSE;
	inDISP_LogPrintf("inCostco_TSP_ProcessScanPay inResult = [%d]", inResult);
	return (inResult);
}

int LoadCreditScanHost(void)
{
        int i = 0;
        char szHostEnable[2 + 1];
        char szHostName[8 + 1];
        
	inDISP_LogPrintf("LoadCreditScanHost START!");
        
        memset(szHostName, 0x00, sizeof(szHostName));
        inGetHostLabel(szHostName);

        for (i = 0 ;; i ++)
        {
                if (inLoadHDTRec(i) < 0)
                        break;

                if (!memcmp(szHostName, _HOST_NAME_CREDIT_MAINLY_USE_, strlen(_HOST_NAME_CREDIT_MAINLY_USE_)))
                {
                        memset(szHostEnable, 0x00, 1);
                        inGetHostEnable(szHostEnable);
                        
                        if (szHostEnable[0] != 'Y')
                        {
                                inDISP_LogPrintf(("LoadCreditScanHost ERROR"));
                                return VS_ERROR;
                        }
                        else
                        {
                                inDISP_LogPrintf(("LoadCreditScanHost SUCCESS"));
                                inDISP_LogPrintf("LoadCreditScanHost END");
                                return VS_SUCCESS;
                        }
                }
        }
        
        inDISP_LogPrintf("Not Find");
	inDISP_LogPrintf("LoadCreditScanHost Error");
        return (VS_ERROR);
}

int LoadRedeemScanHost(void)
{
        int i = 0;
        char szHostEnable[2 + 1];
        char szHostName[8 + 1];
        
	inDISP_LogPrintf("LoadRedeemScanHost Start!");
        
        memset(szHostName, 0x00, sizeof(szHostName));
        inGetHostLabel(szHostName);

        for (i = 0 ;; i ++)
        {
                if (inLoadHDTRec(i) < 0)
                        break;

                if (!memcmp(szHostName, _HOST_NAME_REDEEM_, strlen(_HOST_NAME_REDEEM_)))
                {
                        memset(szHostEnable, 0x00, 1);
                        inGetHostEnable(szHostEnable);
                        
                        if (szHostEnable[0] != 'Y')
                        {
                                inDISP_LogPrintf(("LoadRedeemScanHost ERROR"));
                                return VS_ERROR;
                        }
                        else
                        {
                                inDISP_LogPrintf(("LoadRedeemScanHost SUCCESS"));
                                inDISP_LogPrintf("LoadRedeemScanHost END");
                                return VS_SUCCESS;
                        }
                }
        }
        
        inDISP_LogPrintf("Not Find");
	inDISP_LogPrintf("LoadRedeemScanHost Error");
        return (VS_ERROR);
}

int LoadInstScanHost(void)
{
        int i = 0;
        char szHostEnable[2 + 1];
        char szHostName[8 + 1];
        
	inDISP_LogPrintf("LoadInstScanHost Start!");
        
        memset(szHostName, 0x00, sizeof(szHostName));
        inGetHostLabel(szHostName);

        for (i = 0 ;; i ++)
        {
                if (inLoadHDTRec(i) < 0)
                        break;

                if (!memcmp(szHostName, _HOST_NAME_INST_, strlen(_HOST_NAME_INST_)))
                {
                        memset(szHostEnable, 0x00, 1);
                        inGetHostEnable(szHostEnable);
                        
                        if (szHostEnable[0] != 'Y')
                        {
                                inDISP_LogPrintf(("LoadInstScanHost ERROR"));
                                return VS_ERROR;
                        }
                        else
                        {
                                inDISP_LogPrintf(("LoadInstScanHost SUCCESS"));
                                inDISP_LogPrintf("LoadInstScanHost END");
                                return VS_SUCCESS;
                        }
                }
        }
        
        inDISP_LogPrintf("Not Find");
	inDISP_LogPrintf("LoadInstScanHost Error");
        return (VS_ERROR);
}

int inCostco_ScanPay_Online_Flow(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = 0;
        int	inHexBufferLen = 0;
        char	szHexBuffer[32 + 1] = {0};
        char	szExpDateCheck[2 + 1] = {0};
        char	szExpireDate[4 + 1] = {0};
        
        inDISP_LogPrintf("inCostco_ScanPay_Online_Flow Start");
        
        inDISP_ClearAll();
        inDISP_PutGraphic(_SCAN_PROCESS_TITLE_, 0, _COORDINATE_Y_LINE_8_2_); /* 第二層顯示 ＜掃碼交易＞ */
        inDISP_ChineseFont("連線處理中", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_ChineseFont("請稍候", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
        
        /* 防呆，連線前先斷線 */
        inCOMM_End(pobTran);
        
        /* 先建立連線參數初始化，兩個Flag都要ON */
        pobTran->fSendToCostcoPayHost = VS_TRUE;
        fLinkCostcoPayHost = VS_TRUE;
        
        /* 連線 */
        inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_); 
        if (inRetVal != VS_SUCCESS)
        {
                inCOMM_End(pobTran);
                pobTran->fSendToCostcoPayHost = VS_FALSE;
                fLinkCostcoPayHost = VS_FALSE;
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_ChineseFont("連線失敗", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
                inDISP_BEEP(1,0);
                inDISP_Wait(2000);
                return (VS_ERROR);
        }
        else if(inRetVal == VS_SUCCESS && ginDebug == VS_TRUE)/* Miyano_test 可以用時註解掉 */
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_ChineseFont("連線成功", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        }
        
        
        //組封包傳送
        pobTran->inActionCode = ACTION_DETOKEN_TRANSACTION; /* 設定交易類別 */

        inRetVal = inCostco_TSP_ProcessOnline(pobTran);
        //斷線
        inCOMM_End(pobTran);
        fLinkCostcoPayHost = VS_FALSE;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

        if (inRetVal == VS_SUCCESS)
        {
                /* 產生TMK */
                inDISP_LogPrintf("ProcessScanPay_TMK");
                inCostco_TSP_Generate_TerminalMasterKey(pobTran);

                /* 產生TK */
                inDISP_LogPrintf("ProcessScanPay_TK");
                inCostco_TSP_Generate_TransactionKey(pobTran, D_HostValue);

                /* 解卡號 */
                inDISP_LogPrintf("ProcessScanPay_PAN");
                memset(szHexBuffer, 0x00, sizeof(szHexBuffer));
                inCostco_TSP_AES_DataDecryption((unsigned char *)pobTran->szTransKey, (unsigned char *)pobTran->szEncryptPan, pobTran->inEncryptPanLen, (unsigned char *)szHexBuffer, &inHexBufferLen);
                inDISP_LogPrintf("ProcessScanPay_PAN SSS, inHexBufferLen");
                inFunc_BCD_to_ASCII(&pobTran->srBRec.szPAN[0], &szHexBuffer[0], inHexBufferLen - szHexBuffer[inHexBufferLen - 1]);

                /* 解有效期 */
                inDISP_LogPrintf("inCB_ERM_ProcessScanPay_ExpireDate");
                memset(szHexBuffer, 0x00, sizeof(szHexBuffer));
                memset(szExpireDate, 0x00, sizeof(szExpireDate));
                inCostco_TSP_AES_DataDecryption((unsigned char *)pobTran->szTransKey, (unsigned char *)pobTran->szEncryptExpireDate, pobTran->inEncryptExpireDateLen, (unsigned char *)szHexBuffer, &inHexBufferLen);
                inFunc_BCD_to_ASCII(&szExpireDate[0], &szHexBuffer[0], inHexBufferLen - szHexBuffer[inHexBufferLen - 1]);
                memcpy(&pobTran->srBRec.szExpDate[0], &szExpireDate[2], 2); /* YY */
                memcpy(&pobTran->srBRec.szExpDate[2], &szExpireDate[0], 2); /* MM */

                inDISP_LogPrintf("pobTran->srBRec.szPAN = [%s]", pobTran->srBRec.szPAN);
                inDISP_LogPrintf("pobTran->srBRec.szExpDate = [%s]", pobTran->srBRec.szExpDate);

                inRetVal = inCARD_GetBin(pobTran);
                if (inRetVal == VS_SUCCESS)
                {
                        /* 判斷有效期 */
                        memset(szExpDateCheck, 0x00, sizeof (szExpDateCheck));
                        inGetExpiredDateCheck(szExpDateCheck);
                        
                        if (!memcmp(&szExpDateCheck, "Y", 1))
                        {
                                if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
                                {
                                        inDISP_LogPrintf("CARD ValidateExpDate error");
                                        inRetVal = VS_ERROR;
                                }
                        }
                }
                else
                {
                        memcpy(&pobTran->srBRec.szExpDate[2], &szExpireDate[0], 2); /* MM */
                        inDISP_LogPrintf("inCARD_GetBin error = [%d]", inRetVal);
                }
        }
        else
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_ChineseFont("取得卡號流程失敗", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
                inDISP_BEEP(1,0);
                inDISP_Wait(2000);
        }
        
        inDISP_LogPrintf("inCostco_ScanPay_Online_Flow End");
        return inRetVal;
}

int inCostco_Pack_Object_DetokenTransaction(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	inCnt = 0;
	int	i, inPacketCnt = 0, inSubPacketCnt = 0;
	char	szSubPack[150 + 1];
	char	szPacket[100 + 1];


	inDISP_LogPrintf("inCostco_Pack_Object_DetokenTransaction(inCode = %d) START!!", pobTran->inActionCode);

	memset(szPacket, 0x00, sizeof(szPacket));
//	/* TAG_21 */
//	szPacket[inPacketCnt ++] = 0x21;
//	szPacket[inPacketCnt ++] = 0x04;
//	memcpy(&szPacket[inPacketCnt], "\x00\x00\x00\x09", 4);
//	inPacketCnt += 4;

	/* TAG_bf10 Detoken Object VO */
	szPacket[inPacketCnt ++] = 0xbf;
	szPacket[inPacketCnt ++] = 0x10;

	/* Pack ffbf8001 Token to detoken */
	memset(szSubPack, 0x00, sizeof(szSubPack));
	szSubPack[inSubPacketCnt ++] = 0xff;
	szSubPack[inSubPacketCnt ++] = 0xbf;
	szSubPack[inSubPacketCnt ++] = 0x80;
	szSubPack[inSubPacketCnt ++] = 0x01;
	szSubPack[inSubPacketCnt ++] = pobTran->inQrDataLen;
	memcpy(&szSubPack[inSubPacketCnt], pobTran->szQrData, pobTran->inQrDataLen);
	inSubPacketCnt += pobTran->inQrDataLen;

	szPacket[inPacketCnt ++] = inSubPacketCnt;
	memcpy(&szPacket[inPacketCnt], szSubPack, inSubPacketCnt);
	inPacketCnt += inSubPacketCnt;

	inCnt = inPacketCnt;
	memcpy(&uszPackBuf[0], &szPacket[0], inCnt);

	inDISP_LogPrintf("inCostco_Pack_Object_DetokenTransaction(%ld) END!!", inCnt);
	return (inCnt);
}

int inCostco_TSP_AES_DataDecryption(unsigned char *szKey, unsigned char *szCipherText, int inCipherTextLen, unsigned char *szPlainText, int *inPlainTextLen)
{
	EVP_CIPHER_CTX ctx;

	int	inLen = 0;
	int	inCipherBlockSize, inCipherKeyLength, inCipherIvLength;

	unsigned char szIV[] =
	{ /* Only need 16 bytes... */
		0xa5, 0x83, 0xb4, 0x1b, 0x1b, 0x5a, 0x4b, 0xf2,
		0xc7, 0x0c, 0xf9, 0xdf, 0xed, 0xc6, 0x2c, 0xbc, 0
	};

	memset(szPlainText, 0x00, sizeof(szPlainText));
//	memset(szCipherText, 0x00, 33);
//	memcpy(szCipherText, "\xb5\xe4\x31\xd3\xcf\x93\x1c\x2d\xb1\xaa\xe4\xdc\x8a\x21\xbe\xa8", 16);
//	inCipherTextLen = 16;
	inDISP_LogPrintf("inCostco_TSP_AES_DataDecryption Len =[%d]", inCipherTextLen);

	vdCostco_TSP_PrintHexToString(szCipherText);

	EVP_CIPHER_CTX_init(&ctx);
	EVP_DecryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, (const unsigned char *)szKey, (const unsigned char *)szIV);

	/* 關閉padding 功能,預設開啟 */
	EVP_CIPHER_CTX_set_padding(&ctx, 0);

	inCipherBlockSize = EVP_CIPHER_CTX_block_size(&ctx);
	inCipherKeyLength = EVP_CIPHER_CTX_key_length(&ctx);
	inCipherIvLength  = EVP_CIPHER_CTX_iv_length(&ctx);

	inDISP_LogPrintf("INFO(evp_decrypt): Input length: %d", inCipherTextLen);
	inDISP_LogPrintf("INFO(evp_decrypt): block size: %d", inCipherBlockSize);
	inDISP_LogPrintf("INFO(evp_decrypt): key length: %d", inCipherKeyLength);
	inDISP_LogPrintf("INFO(evp_decrypt): IV length:  %d", inCipherIvLength);

  	if ((inCipherKeyLength > 32) || (inCipherIvLength  > 16))
	{
		inDISP_LogPrintf("inCostco_TSP_AES_DataDecryption_Length_ERR");
		return (VS_ERROR);
	}

	/*
	int	EVP_DecryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out,
		int *outl, const unsigned char *in, int inl);
	*/

	if (1 != EVP_DecryptUpdate(&ctx, szPlainText, &inLen, szCipherText, inCipherTextLen))
	{
		inDISP_LogPrintf("inCostco_TSP_AES_DataDecryption_Update_ERR");
		return (VS_ERROR);
	}
	*inPlainTextLen = inLen;

	if (1 != EVP_DecryptFinal_ex(&ctx, szPlainText + inLen, &inLen))
	{
		inDISP_LogPrintf("inCostco_TSP_AES_DataDecryption_Final_ERR");
		return (VS_ERROR);
	}
	*inPlainTextLen += inLen;
	inDISP_LogPrintf("INFO(evp_decrypt): Output length: %d", *inPlainTextLen);

	EVP_CIPHER_CTX_cleanup(&ctx);

	/* Sample Value
		Key 		= 19e6e525ea9156532307991c062b2330b5b5f7638486367dfd79177f247dc63a
		IV  		= a583b41b1b5a4bf2c70cf9dfedc62cbc
		Plaintext	= dddddddd
		Ciphered	= dc549e9454f4702c3987a52589ab54a1 */

	vdCostco_TSP_PrintHexToString1(szPlainText, *inPlainTextLen);

	inDISP_LogPrintf("inCostco_TSP_AES_DataDecryption END");
	return (VS_SUCCESS);
}

/*
Function        : inCostco_GasStation_RS232_ECR_8N1_Initial
Date&Time       : 2023/03/14
Describe        : 
*/
int inCostco_GasStation_RS232_ECR_8N1_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1];
	char		szECRComPort[4 + 1];
	unsigned char	uszParity;
	unsigned char	uszDataBits;
	unsigned char	uszStopBits;
	unsigned long	ulBaudRate;
	unsigned short	usRetVal;
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));
	
	/* 從EDC.Dat抓出哪一個Comport */
	/* inGetECRComPort */
	memset(&szECRComPort, 0x00, sizeof(szECRComPort));
	inGetECRComPort(szECRComPort);
	/* Verifone用handle紀錄，Castle用Port紀錄 */
	if (!memcmp(szECRComPort, "COM1", 4))
		srECROb->srSetting.uszComPort = d_COM1;
	else if (!memcmp(szECRComPort, "COM2", 4))
		srECROb->srSetting.uszComPort = d_COM2;
	if (!memcmp(szECRComPort, "COM3", 4))
		srECROb->srSetting.uszComPort = d_COM3;
	if (!memcmp(szECRComPort, "COM4", 4))
		srECROb->srSetting.uszComPort = d_COM4;
		
	/* BaudRate = 19200 */
	ulBaudRate = 19200;
	
	/* Parity */
	uszParity = 'N';
	
	/* Data Bits */
	uszDataBits = 8;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
        usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

        if (usRetVal != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
                        inDISP_LogPrintf(szDebugMsg);
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
                        inDISP_LogPrintf(szDebugMsg);
                }
                return (VS_ERROR);         
        }
        else
        {
                if (ginDebug == VS_TRUE)
                {
                        inDISP_LogPrintf("inRS232_Open OK");
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
                        inDISP_LogPrintf(szDebugMsg);
                }
        }

	/* 清空接收的buffer */
        inRS232_FlushRxBuffer(srECROb->srSetting.uszComPort);
	
        return (VS_SUCCESS);
}

/*
 * Function        : inCostco_GasStation_RS232_ECR_8N1_Receive_Packet
 * Date&Time       : 2023/04/20
 * Describe        : 處理收銀機傳來的資料
 * Other           : 這支會收兩段收銀機指令，並在第二段收到P01指令後，回傳收銀機相應資料
 * !@#
*/
int inCostco_GasStation_RS232_ECR_8N1_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
        int	inRetVal;
        char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
        char    szTmp[20+1]; 
        
        ECR_TABLE       srECROb2_local;
        ECR_TABLE       *srECROb2;
        ECR_GASSTATION_GLOBAL_TABLE *gsrGastb;
        
        gsrGastb = &gsrECR_GASStation_tb;
        srECROb2 = &srECROb2_local;
        
        inDISP_LogPrintf("inCostco_GasStation_RS232_ECR_8N1_Receive_Packet Start");
        
        /* 防呆 */
        srECROb->srTransDataGas.inTransType = NULL;
        srECROb->srTransDataGas.inFlag_P01 = 0;
        gsrGastb->inFlag_NextState = 0;
        
        memset(gsrGastb->szCommand, 0x00, sizeof(gsrGastb->szCommand));
        memset((char *)srECROb2, 0x00, sizeof(srECROb2));
        
             
        memset(szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */

        inRetVal = inECR_Costco_GasStation_Receive(pobTran, srECROb, szDataBuffer);

        if (inRetVal != VS_SUCCESS)
        {
                return (inRetVal);
        }

        /* 分析資料 */
        inRetVal = inECR_Costco_GasStation_Unpack(pobTran, srECROb, szDataBuffer);

        if (inRetVal != VS_SUCCESS)
        {
                inRS232_ECR_SendError(pobTran, inRetVal);
                return (inRetVal);
        }

        memcpy(gsrGastb->szCommand, srECROb->srTransDataGas.szCommand, strlen(srECROb->srTransDataGas.szCommand));
        
        /* 開機第一次要先發P06，沒發過就Return error */
        if(strlen(gsrECR_GASStation_tb.szPumpNo) == 0)
        {
                memcpy(srECROb->srTransDataGas.szCommand, "Initial", 7);
                inCostco_GasStation_RS232_ECR_8N1_Send_Packet2(pobTran, srECROb);
                return VS_ERROR;
        }

        /* 20230424 Miyano 進入功能或流程放這裡 */
        inCostco_GasStation_Select_TransType(pobTran, srECROb);
        
        /* 送錯指令檔掉 Return，ex:第一階段不送預先授權和授權完成  */
        if(srECROb->srTransDataGas.inTransType == _Costco_PreAuth_ ||
           srECROb->srTransDataGas.inTransType == _Costco_AuthComplete_)
        {
                inDISP_LogPrintf("Wrong TransType (%d)", srECROb->srTransDataGas.inTransType);
                gsrGastb->inFlag_NextState = 0;
                return VS_ERROR;
        }
        
        /* 第一段送 */
        inRetVal = inCostco_GasStation_RS232_ECR_8N1_Send_Packet(pobTran, srECROb);
        if(inRetVal != VS_SUCCESS)
        {
                inRS232_ECR_SendError(pobTran, inRetVal);
                gsrGastb->inFlag_NextState = 0;
                return (inRetVal);
        }
        
        /* ----------------20230424 執行第一段ECR發動的指令 ---------------- */        
        if(srECROb->srTransDataGas.inTransType != NULL)
        {
                inRetVal = inCostco_GasStation_Run_TransType(pobTran, srECROb);
                inDISP_LogPrintf("Miyano Flag inRetVal = %d", inRetVal);
                /* 處理ECR發動的指令 */
                inCostco_GasStation_Handle_CommandResult(pobTran, srECROb, inRetVal);
        }
        
        /* 收P01 並回應ECR，如果已經發P01就跳過 */
        if(memcmp(srECROb->srTransDataGas.szCommand, "P01", 3))
        {
                while(1)
                {
                        inRetVal = inCostco_GasStation_ECR_2nd_Receive_P01(pobTran, srECROb2);
                        if(inRetVal == VS_TIMEOUT)
                        {
                                inDISP_LogPrintf("Wait P01 Timeout");
                                gsrGastb->inFlag_NextState = 0;
                                return VS_TIMEOUT;
                        }
                        else if(inRetVal == VS_ERROR)
                        {
                                memset(szTmp, 0x00, sizeof(szTmp));
                                memcpy(szTmp, srECROb->srTransDataGas.szCommand, strlen(srECROb->srTransDataGas.szCommand));
                                memcpy(srECROb->srTransDataGas.szCommand, "Busy", 4);
                                inCostco_GasStation_RS232_ECR_8N1_Send_Packet2(pobTran, srECROb);
                                memset(srECROb->srTransDataGas.szCommand, 0x00, sizeof(srECROb->srTransDataGas.szCommand));
                                memcpy(srECROb->srTransDataGas.szCommand, szTmp, strlen(szTmp));
                        }
                        else if(inRetVal == VS_SUCCESS)
                        {
                                break;
                        }
                }
                inCostco_GasStation_RS232_ECR_8N1_Send_Packet2(pobTran, srECROb);
        }
        
        inDISP_LogPrintf("inCostco_GasStation_RS232_ECR_8N1_Receive_Packet End");
        return (VS_SUCCESS);
}

/* 20230411 Miyano 好市多加油站，等著收P01用，收到資料 or Timeout 再 Break */
int inCostco_GasStation_ECR_2nd_Receive_P01(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
        int	inRetVal = VS_ERROR;
        char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */

        inDISP_LogPrintf("inCostco_GasStation_ECR_2nd_Receive_P01 Start");
        
        memset(szDataBuffer, 0x00, sizeof(szDataBuffer));
        
        srECROb->srTransDataGas.inFlag_P01 = 1;
        if(inECR_Costco_GasStation_Receive(pobTran, srECROb, szDataBuffer) == VS_SUCCESS)
        {
                /* 開始分析資料，收到P01才算 */
                inRetVal = inECR_Costco_GasStation_Unpack_P01(pobTran, srECROb, szDataBuffer);
        }
        else if(strlen(szDataBuffer) == 0)
        {
                inRetVal = VS_TIMEOUT;
        }

        inDISP_LogPrintf("inCostco_GasStation_ECR_2nd_Receive_P01 End");
        return (inRetVal);
}

int inCostco_GasStation_RS232_ECR_8N1_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
        int	inRetVal = VS_ERROR;
        char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */

        /* 如果已經回過ECR就不再回 */
        if (srECROb->srTransData.uszIsResponce == VS_TRUE)
	        return (VS_SUCCESS);

        /* 初始化 */
        memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
        inRetVal = inECR_Costco_GasStation_Pack(pobTran, srECROb, &szDataBuf);

        if (inRetVal == VS_ERROR)
        {
	        return (VS_ERROR);
        }

/* ---------------------傳送電文--------------------------------------------- */
        inRetVal = inECR_Send(pobTran, srECROb, &szDataBuf, strlen(szDataBuf));

        if (inRetVal != VS_SUCCESS)
        {
	        return (inRetVal);
        }
	
        return (VS_SUCCESS);
}

int inCostco_GasStation_RS232_ECR_8N1_Send_Packet2(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
        int	inRetVal = VS_ERROR;
        char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */

        /* 如果已經回過ECR就不再回 */
        if (srECROb->srTransData.uszIsResponce == VS_TRUE)
	        return (VS_SUCCESS);

        /* 初始化 */
        memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
        inRetVal = inECR_Costco_GasStation_Pack2(pobTran, srECROb, &szDataBuf);

        if (inRetVal == VS_ERROR)
        {
	        return (VS_ERROR);
        }

/* ---------------------傳送電文--------------------------------------------- */
        inRetVal = inECR_Send(pobTran, srECROb, &szDataBuf, strlen(szDataBuf));

        if (inRetVal != VS_SUCCESS)
        {
	        return (inRetVal);
        }
	
        return (VS_SUCCESS);
}

/* Costco加油站不能用這支，要根據不同交易別，不同情況Send不同Error */
int inCostco_GasStation_RS232_ECR_8N1_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
        int	inRetVal;
        char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
        inDISP_LogPrintf("inCostco_GasStation_RS232_ECR_8N1_Send_Error Start");
        /* 如果已經回過ECR就不再回 */
        if (srECROb->srTransData.uszIsResponce == VS_TRUE)
	        return (VS_SUCCESS);

        /* 初始化 */
        memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */

        inRetVal = inECR_Costco_GasStation_Pack(pobTran, srECROb, &szDataBuf);

        if (inRetVal != VS_SUCCESS)
        {
	        return (inRetVal);
        }

        
/* ---------------------傳送電文--------------------------------------------- */
        inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, strlen(szDataBuf));

        if (inRetVal != VS_SUCCESS)
        {
	        return (inRetVal);
        }

        /* 標示已送給ECR回覆電文 */
        srECROb->srTransData.uszIsResponce = VS_TRUE;

        inDISP_LogPrintf("inCostco_GasStation_RS232_ECR_8N1_Send_Error End");
        return (VS_SUCCESS);
}

int inCostco_GasStation_Select_TransType(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
        int	inRetVal = VS_ERROR;
        char    szTransType_Buffer[2+1] = {0};
        
        inDISP_LogPrintf("inCostco_GasStation_Select_TransType Start");
        
        if(!memcmp(srECROb->srTransDataGas.szCommand, "P01", 3))    /* Poll */
        {
                /* P01 不做TransType */
                srECROb->srTransDataGas.inTransType = NULL;
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P02", 3))    /* 授權完成 */
        {
                pobTran->srBRec.lnOrgInvNum = atoi(srECROb->srTransDataGas.szInvoice);
                /* 找到符合pobTran->srBRec.lnOrgInvNum的bkey記錄  */
                inBATCH_GetTransRecord(pobTran);
                inDISP_LogPrintf("Miyano Card Number = %s\n", srECROb->srTransDataGas.szPan);
                inDISP_LogPrintf("pobTran Card Number = %s\n", pobTran->srBRec.szPAN);
                inDISP_LogPrintf("Miyano Approve No = %s\n", srECROb->srTransDataGas.szApproveNo);
                inDISP_LogPrintf("pobTran Approve No = %s\n", pobTran->srBRec.szAuthCode);
                pobTran->inTransactionCode = _PRE_COMP_;
                pobTran->srBRec.inCode = _PRE_COMP_;
                srECROb->srTransDataGas.inTransType = _Costco_AuthComplete_;
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P04", 3))
        {
                if(!memcmp(srECROb->srTransDataGas.szEDCMode, "B", 1))
                {
                        inDISP_LogPrintf("Settle");  /* 結帳 */
                        pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;
			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;
                        srECROb->srTransDataGas.inTransType = _Costco_Settle_;
                }
                else if(!memcmp(srECROb->srTransDataGas.szEDCMode, "E", 1))
                {
                        if(!memcmp(srECROb->srTransDataGas.uszArgement1, "0", 1))
                        {
                                inDISP_LogPrintf("Read Card");  /* 過卡(信用卡) */
                                srECROb->srTransDataGas.inTransType = _Costco_ReadCard_;
                                pobTran->inTransactionCode = _PRE_AUTH_;
                                gsrECR_GASStation_tb.inFlag_NextState = 1;
                        }
                        else if(atoi(srECROb->srTransDataGas.uszArgement1) > 0)
                        {
                                inDISP_LogPrintf("PreAuth Wait");   /* 預先授權 */
                                srECROb->srTransDataGas.inTransType = _Costco_PreAuth_;
                                pobTran->inTransactionCode = _PRE_AUTH_;
                                pobTran->srBRec.lnTxnAmount = atoi(srECROb->srTransDataGas.uszArgement1);
                                gsrECR_GASStation_tb.inFlag_NextState = 1;
                        }
                }
                else if(!memcmp(srECROb->srTransDataGas.szEDCMode, "R", 1))
                {
                        inDISP_LogPrintf("Reboot");  /* 重新開機 */
                        srECROb->srTransDataGas.inTransType = _Costco_Reboot_;
                }
        }
        if(!memcmp(srECROb->srTransDataGas.szCommand, "P06", 3))    /* 開機初始 */
        {
                /* P06 不做TransType */
                srECROb->srTransDataGas.inTransType = NULL;
        }
        else if(!memcmp(srECROb->srTransDataGas.szCommand, "P08", 3))    /* 掃碼 */
        {
                pobTran->srBRec.lnOrgInvNum = atoi(srECROb->srTransDataGas.szInvoice);
                pobTran->inTransactionCode = _PRE_AUTH_;
                srECROb->srTransDataGas.inTransType = _Costco_ScanPay_;
                gsrECR_GASStation_tb.inFlag_NextState = 1;
        }
        
        if(srECROb->srTransDataGas.inTransType != NULL)
        {
                memset(szTransType_Buffer, 0x00, sizeof(szTransType_Buffer));
                sprintf(szTransType_Buffer, "%02d", srECROb->srTransDataGas.inTransType);
                memcpy(gsrECR_GASStation_tb.szCommand, szTransType_Buffer, 2);
        }

        inDISP_LogPrintf("inCostco_GasStation_Select_TransType End");
        
        return (VS_SUCCESS);
}

int inCostco_GasStation_Run_TransType(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
        int	inRetVal = 0;
        int     inTemp = 0;
        
        ACCUM_TOTAL_REC srAccumRec;
        
        inDISP_LogPrintf("inCostco_GasStation_Run_TransType Start(%d)", srECROb->srTransDataGas.inTransType);
        
        switch(srECROb->srTransDataGas.inTransType)
        {
                case _Costco_ReadCard_:
                        inDISP_LogPrintf("inTransactionCode = %d", pobTran->inTransactionCode);
                        inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_GET_CARD_FIELDS_CTLS_);
                        if(inRetVal != VS_SUCCESS)
                        {
                                inDISP_LogPrintf("inTransactionCode = %d", pobTran->inTransactionCode);
                                inDISP_LogPrintf("Read Card Fail");
                        }
                        break;
                case _Costco_PreAuth_:
                        inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_CHECK_PAN_EXPDATE_);
                        if(inRetVal == VS_SUCCESS)
                        {
                                inRetVal = inFUBON_RunTRT(pobTran, _TRT_PRE_AUTH_CTLS_);
                        }
                        
                        if(inRetVal != VS_SUCCESS)
                        {
                                inDISP_LogPrintf("PreAuth Fail");
                        }
                        break;
                case _Costco_AuthComplete_:
                        inTemp = pobTran->inRunOperationID;
                        pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
                        inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
                        if(inRetVal != VS_SUCCESS)
                        {
                                inDISP_LogPrintf("Auth Complete Error");
                        }
                        else
                        {
                                inRetVal = inFUBON_RunTRT(pobTran, _TRT_PRE_COMP_CTLS_);
                        }
                        pobTran->inRunOperationID = inTemp;
                        break;
                case _Costco_Settle_:
                        inTemp = pobTran->inRunOperationID;
                        inACCUM_GetRecord(pobTran, &srAccumRec);
                        srECROb->srTransDataGas.inTransTotalCount = srAccumRec.lnTotalCreditCount;
                        srECROb->srTransDataGas.inTransTotalAmount = srAccumRec.llTotalCreditAmount;
                        
                        inDISP_LogPrintf("inTransTotalCount = %d", srECROb->srTransDataGas.inTransTotalCount);
                        inDISP_LogPrintf("inTransTotalAmount = %d", srECROb->srTransDataGas.inTransTotalAmount);
                        
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);

                        if(inRetVal != VS_SUCCESS)
                        {
                                inDISP_LogPrintf("Settle Error");
                        }
                        else
                        {
                                inRetVal = inFUBON_RunTRT(pobTran, pobTran->inRunTRTID);
                        }
                        pobTran->inRunOperationID = inTemp;
                        break;
                case _Costco_Reboot_:
                        inFunc_Reboot();
                        break;
                case _Costco_ScanPay_:
                        inRetVal = inFLOW_RunFunction(pobTran, _COSTCO_SCANPAY_);
                        if(inRetVal != VS_SUCCESS)
                        {
                                inDISP_LogPrintf("inTransactionCode = %d", pobTran->inTransactionCode);
                                inDISP_LogPrintf("CostcoPay Fail");
                        }
                        break;
                default:
                        inDISP_LogPrintf("Default");
                        inRetVal = VS_SUCCESS;
                        break;
        }
        
        inDISP_LogPrintf("inCostco_GasStation_Run_TransType End");
        return (inRetVal);
}

int inCostco_GasStation_Handle_CommandResult(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult)
{
        int	inRetVal = 0;
        
        inDISP_LogPrintf("inCostco_GasStation_Handle_CommandResult Start(%d)", srECROb->srTransDataGas.inTransType);
        
        switch(srECROb->srTransDataGas.inTransType)
        {
                case _Costco_ReadCard_:
                        inDISP_LogPrintf("_Costco_ReadCard_");
                        inDISP_LogPrintf("inCommandResult = %d", inCommandResult);
                        inCostco_GasStation_Handle_ReadCard(pobTran, srECROb, inCommandResult);
                        break;
                case _Costco_PreAuth_:
                        inDISP_LogPrintf("_Costco_PreAuth_");
                        if(inCommandResult != VS_SUCCESS)
                        {
                                inCostco_GasStation_Handle_PreAuth_Complete_ERR(pobTran, srECROb, inCommandResult);
                                memset(gsrECR_GASStation_tb.szCommand, 0x00, sizeof(gsrECR_GASStation_tb.szCommand));
                                memcpy(gsrECR_GASStation_tb.szCommand, _Costco_szCommand_PreAuth_Complete_Err, 2);
                        }
                        else
                        {
                                inCostco_GasStation_Handle_PreAuth(pobTran, srECROb, inCommandResult);
                                inDISP_LogPrintf("srECROb->srTransDataGas.lnAmount = %ld", srECROb->srTransDataGas.lnAmount);

                        }
                        break;
                case _Costco_AuthComplete_:
                        inDISP_LogPrintf("_Costco_AuthComplete_");
                        if(inCommandResult != VS_SUCCESS)
                        {
                                inCostco_GasStation_Handle_PreAuth_Complete_ERR(pobTran, srECROb, inCommandResult);
                                memset(gsrECR_GASStation_tb.szCommand, 0x00, sizeof(gsrECR_GASStation_tb.szCommand));
                                memcpy(gsrECR_GASStation_tb.szCommand, _Costco_szCommand_PreAuth_Complete_Err, 2);
                        }
                        else
                        {
                                inCostco_GasStation_Handle_AuthComplete(pobTran, srECROb, inCommandResult);
                        }
                        break;
                case _Costco_Settle_:
                        inDISP_LogPrintf("_Costco_Settle_");
                        inCostco_GasStation_Handle_Settle(pobTran, srECROb, inCommandResult);
                        break;
                case _Costco_ScanPay_:
                        inDISP_LogPrintf("_Costco_ScanPay_");
                        inCostco_GasStation_Handle_ScanPay(pobTran, srECROb, inCommandResult);
                        break;
                default:
                        inDISP_LogPrintf("Default");
                        inRetVal = VS_SUCCESS;
                        break;
        }
        
        inDISP_LogPrintf("inCostco_GasStation_Handle_CommandResult End");
        return (inRetVal);
}


int inCostco_GasStation_Handle_ReadCard(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult)
{
        int i = 0;
        inDISP_LogPrintf("inCostco_GasStation_Handle_ReadCard Start");
       
        /* 加油泵號碼 */
        memcpy(srECROb->srTransDataGas.szPumpNo, gsrECR_GASStation_tb.szPumpNo,  strlen(gsrECR_GASStation_tb.szPumpNo));
        /* Response Code */
        switch(inCommandResult)
        {
                case VS_SUCCESS:
                        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				memcpy(srECROb->srTransDataGas.szResponse, "3", 1);
                        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				memcpy(srECROb->srTransDataGas.szResponse, "0", 1);
                        else
                                inDISP_LogPrintf("Miyano Flag other");
                        break;
                case VS_ERROR:
                case VS_TIMEOUT:
                        memcpy(srECROb->srTransDataGas.szResponse, "1", 1);
                        break;
                default:
                        inDISP_LogPrintf("inCommandResult = %d", inCommandResult);
                        break;
        }
        inDISP_LogPrintf("inCommandResult = %d", inCommandResult);
        
        /* 暫時，卡別 */
        if(!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, 4) ||
           !memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, 10) ||
           !memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, 3) )
        {
                memcpy(srECROb->srTransDataGas.szCardType, "02", 2);
        }
        else    /* 暫時，待改 */
        {
                memcpy(srECROb->srTransDataGas.szCardType, "XX", 2);
        }
        
        inDISP_LogPrintf("szCardLabel = %s", pobTran->srBRec.szCardLabel);
        
        /* 卡號 *//* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
        memcpy(srECROb->srTransDataGas.szPan, pobTran->srBRec.szPAN, 20);
        if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
        {
                for (i = 3; i < (strlen(srECROb->srTransDataGas.szPan) - 5); i ++)
                        srECROb->srTransDataGas.szPan[i] = 0x2A;
        }
        else
        {
                for (i = 6; i < (strlen(srECROb->srTransDataGas.szPan) - 4); i ++)
                        srECROb->srTransDataGas.szPan[i] = 0x2A;
        }
        inDISP_LogPrintf("szPan = %s", srECROb->srTransDataGas.szPan);
        
        /* 卡片有效期 */
        memcpy(srECROb->srTransDataGas.szCardExpDate, pobTran->srBRec.szExpDate, 4);
        
        /* 暫時，Present Amount */
        memcpy(srECROb->srTransDataGas.szPresetAmount, "00003000", 8);
        
        /* 暫時，Transaction Type */
        memcpy(srECROb->srTransDataGas.szTransactionType, "00", 2);
        
        /* 卡片種類 */
        memcpy(srECROb->srTransDataGas.szCardKind, pobTran->srBRec.szCardLabel, 20); 
        
        /* 過卡方式 */
        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                memcpy(srECROb->srTransDataGas.szWayPassCard, "C", 1);
        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                memcpy(srECROb->srTransDataGas.szWayPassCard, "R", 1);
        else    /* 視為磁條 */
                memcpy(srECROb->srTransDataGas.szWayPassCard, "W", 1);
        
        inDISP_LogPrintf("inCostco_GasStation_Handle_ReadCard End");
        return (VS_SUCCESS);
}

int inCostco_GasStation_Handle_PreAuth(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult)
{
        char    szTmp[100+1] = {0};
        int     i = 0;
        
        inDISP_LogPrintf("inCostco_GasStation_Handle_PreAuth Start");
       
        /* 加油泵號碼 */
        memcpy(srECROb->srTransDataGas.szPumpNo, gsrECR_GASStation_tb.szPumpNo,  strlen(gsrECR_GASStation_tb.szPumpNo));
        /* Response Code */
        switch(inCommandResult)
        {
                case VS_SUCCESS:
                        memcpy(srECROb->srTransDataGas.szResponse, "0", 1);
                        break;
                case VS_ERROR:
                case VS_TIMEOUT:
                        memcpy(srECROb->srTransDataGas.szResponse, "1", 1);
                        break;
                default:
                        inDISP_LogPrintf("inCommandResult = %d", inCommandResult);
                        break;
        }

        /* 暫時，卡別 */
        if(!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, 4) ||
           !memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, 10) ||
           !memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, 3) )
        {
            memcpy(srECROb->srTransDataGas.szCardType, "02", 2);
        }

        /* 卡號 */
        memcpy(srECROb->srTransDataGas.szPan, pobTran->srBRec.szPAN, 20);
        if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
        {
                for (i = 3; i < (strlen(srECROb->srTransDataGas.szPan) - 5); i ++)
                        srECROb->srTransDataGas.szPan[i] = 0x2A;
        }
        else
        {
                for (i = 6; i < (strlen(srECROb->srTransDataGas.szPan) - 4); i ++)
                        srECROb->srTransDataGas.szPan[i] = 0x2A;
        }
        
        /* 卡片有效期 */
        memcpy(srECROb->srTransDataGas.szCardExpDate, pobTran->srBRec.szExpDate, 4);
        
        /* 暫時，Present Amount */
        memcpy(srECROb->srTransDataGas.szPresetAmount, "00003000", 8);
        
        /* 暫時，Transaction Type */
        memcpy(srECROb->srTransDataGas.szTransactionType, "00", 2);
        
        /* Trans Amount */
        srECROb->srTransDataGas.lnAmount = pobTran->srBRec.lnTxnAmount;
        inDISP_LogPrintf("lnAmount = %ld", pobTran->srBRec.lnTxnAmount);
        inDISP_LogPrintf("srECROb->srTransDataGas.lnAmount = %ld", srECROb->srTransDataGas.lnAmount);

        /* 授權碼 */
        memcpy(srECROb->srTransDataGas.szApproveNo, pobTran->srBRec.szAuthCode, 6);
        
        /* TID */
        memset(szTmp, 0x00, sizeof(szTmp));
        inGetTerminalID(szTmp);
        memcpy(srECROb->srTransDataGas.szTID, szTmp, 8);
        inDISP_LogPrintf("szTID = %s", srECROb->srTransDataGas.szTID);
 
        /* MID */
        memset(szTmp, 0x00, sizeof(szTmp));
        inGetMerchantID(szTmp);
        memcpy(srECROb->srTransDataGas.szMID, szTmp, 15);
        inDISP_LogPrintf("szMID = %s", srECROb->srTransDataGas.szMID);

        /* 批次號碼 */
        memset(szTmp, 0x00, sizeof(szTmp));
        sprintf(szTmp, "%ld", pobTran->srBRec.lnBatchNum);
        memcpy(srECROb->srTransDataGas.szBatch, szTmp, 6);
        inDISP_LogPrintf("szBatch = %s", srECROb->srTransDataGas.szBatch);

        /* 調閱編號 */
        memset(szTmp, 0x00, sizeof(szTmp));
        inGetInvoiceNum(szTmp);
        memcpy(srECROb->srTransDataGas.szInvoice, szTmp, 6);
        inDISP_LogPrintf("szInvoice = %s", srECROb->srTransDataGas.szInvoice);

        /* 城市名 */
        memset(szTmp, 0x00, sizeof(szTmp));
        inGetCityName(szTmp);
        memcpy(srECROb->srTransDataGas.szCity, szTmp, 21);

        /* 卡片種類 */
        memcpy(srECROb->srTransDataGas.szCardKind, pobTran->srBRec.szCardLabel, 20); 
        
        /* 過卡方式 */
        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                memcpy(srECROb->srTransDataGas.szWayPassCard, "C", 1);
        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                memcpy(srECROb->srTransDataGas.szWayPassCard, "R", 1);
        else    /* 視為磁條 */
                memcpy(srECROb->srTransDataGas.szWayPassCard, "W", 1);
        
        /* TxRespCode */
        memcpy(srECROb->srTransDataGas.szTxRespCode, "5", 1);

        /* 暫時，看起來沒寫加油站，待補，電子發票載具 */
//        memset(szTmp, 0x00, sizeof(szTmp));
//        inECR_PACK_NewHashCardNumber(pobTran, &szTmp ,50);
//        memcpy((char *)srECROb->srTransDataGas.szEInvoiceCarrier, szTmp, 50);
//        
//        inDISP_LogPrintf("EI = %s", srECROb->srTransDataGas.szEInvoiceCarrier);
//        inDISP_Wait(300);
        
        memset((char *)srECROb->srTransDataGas.szEInvoiceCarrier, 0x24, 50); 

        /* TVR */
        memcpy((char *)srECROb->srTransDataGas.usz95_TVR, pobTran->srEMVRec.usz95_TVR, 6);
        
        /* TSI */
        memcpy((char *)srECROb->srTransDataGas.usz9B_TSI, pobTran->srEMVRec.usz9B_TSI, 2);
        
        /* AIP */
        memcpy((char *)srECROb->srTransDataGas.usz82_AIP, pobTran->srEMVRec.usz82_AIP, 2);
        
        /* CID */
        memcpy((char *)srECROb->srTransDataGas.usz9F27_CID, pobTran->srEMVRec.usz9F27_CID, 2);
        
        /* 9F26 */
        memcpy((char *)srECROb->srTransDataGas.usz9F26_ApplCryptogram, pobTran->srEMVRec.usz9F26_ApplCryptogram, 8);
        
        inDISP_LogPrintf("inCostco_GasStation_Handle_PreAuth End");
        return (VS_SUCCESS);
}

int inCostco_GasStation_Handle_AuthComplete(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult)
{
        char    szTmp[100+1] = {0};
        int     i = 0;
        
        inDISP_LogPrintf("inCostco_GasStation_Handle_AuthComplete Start");
       
        /* 加油泵號碼 */
        memcpy(srECROb->srTransDataGas.szPumpNo, gsrECR_GASStation_tb.szPumpNo,  strlen(gsrECR_GASStation_tb.szPumpNo));
        /* EDC狀態 */
        if(pobTran->srBRec.lnTxnAmount == 0)
                memcpy(srECROb->srTransDataGas.szEDCStatus, "Z", 1);
        else if(pobTran->srBRec.lnTxnAmount > 0)
                memcpy(srECROb->srTransDataGas.szEDCStatus, "C", 1);
        else
                memcpy(srECROb->srTransDataGas.szEDCStatus, "E", 1);    /* 暫時，防呆測試 */
        
        /* 暫時，卡別 */
        if(!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, 4) ||
           !memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, 10) ||
           !memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, 3) )
        {
            memcpy(srECROb->srTransDataGas.szCardType, "02", 2);
        }
        
        /* 卡號 */
        memcpy(srECROb->srTransDataGas.szPan, pobTran->srBRec.szPAN, 20);
        if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_U_CARD_, strlen(_CARD_TYPE_U_CARD_)) == 0)
        {
                for (i = 3; i < (strlen(srECROb->srTransDataGas.szPan) - 5); i ++)
                        srECROb->srTransDataGas.szPan[i] = 0x2A;
        }
        else
        {
                for (i = 6; i < (strlen(srECROb->srTransDataGas.szPan) - 4); i ++)
                        srECROb->srTransDataGas.szPan[i] = 0x2A;
        }
        
        /* 卡片有效期 */
        memcpy(srECROb->srTransDataGas.szCardExpDate, pobTran->srBRec.szExpDate, 4);
        
        /* 暫時，Present Amount */
        memcpy(srECROb->srTransDataGas.szPresetAmount, "00003000", 8);
        
        /* 暫時，Transaction Type */
        memcpy(srECROb->srTransDataGas.szTransactionType, "00", 2);
        
        /* Trans Amount */
        srECROb->srTransDataGas.lnAmount = pobTran->srBRec.lnTxnAmount;
        
        /* 授權碼 */
        memcpy(srECROb->srTransDataGas.szApproveNo, pobTran->srBRec.szAuthCode, 6);
        
        /* TID */
        memset(szTmp, 0x00, sizeof(szTmp));
        inGetTerminalID(szTmp);
        memcpy(srECROb->srTransDataGas.szTID, szTmp, 8);
        
        /* MID */
        memset(szTmp, 0x00, sizeof(szTmp));
        inGetMerchantID(szTmp);
        memcpy(srECROb->srTransDataGas.szMID, szTmp, 15);
        
        /* 批次號碼 */
        memset(szTmp, 0x00, sizeof(szTmp));
        sprintf(szTmp, "%ld", pobTran->srBRec.lnBatchNum);
        memcpy(srECROb->srTransDataGas.szBatch, szTmp, 6);
        inDISP_LogPrintf("szBatch = %s", srECROb->srTransDataGas.szBatch);
        inDISP_Wait(300);
        
        /* 調閱編號 */
        memset(szTmp, 0x00, sizeof(szTmp));
        sprintf(szTmp, "%ld", pobTran->srBRec.lnInvNum);
        memcpy(srECROb->srTransDataGas.szInvoice, szTmp, 6); 
        inDISP_LogPrintf("szInvoice = %s", srECROb->srTransDataGas.szInvoice);
        inDISP_Wait(300);
        
        /* 城市名 */
        memset(szTmp, 0x00, sizeof(szTmp));
        inGetCityName(szTmp);
        memcpy(srECROb->srTransDataGas.szCity, szTmp, 21);
        
        /* 卡片種類 */
        memcpy(srECROb->srTransDataGas.szCardKind, pobTran->srBRec.szCardLabel, 20); 
        
        /* 過卡方式 */
        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                memcpy(srECROb->srTransDataGas.szWayPassCard, "C", 1);
        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                memcpy(srECROb->srTransDataGas.szWayPassCard, "R", 1);
        else    /* 視為磁條 */
                memcpy(srECROb->srTransDataGas.szWayPassCard, "W", 1);

        /* TVR */
        memcpy((char *)srECROb->srTransDataGas.usz95_TVR, pobTran->srEMVRec.usz95_TVR, 6);
        
        /* TSI */
        memcpy((char *)srECROb->srTransDataGas.usz9B_TSI, pobTran->srEMVRec.usz9B_TSI, 2);
        
        /* AIP */
        memcpy((char *)srECROb->srTransDataGas.usz82_AIP, pobTran->srEMVRec.usz82_AIP, 2);
        
        /* CID */
        memcpy((char *)srECROb->srTransDataGas.usz9F27_CID, pobTran->srEMVRec.usz9F27_CID, 2);
        
        /* 9F26 */
        memcpy((char *)srECROb->srTransDataGas.usz9F26_ApplCryptogram, pobTran->srEMVRec.usz9F26_ApplCryptogram, 8);
        
        inDISP_LogPrintf("inCostco_GasStation_Handle_AuthComplete End");
        return (VS_SUCCESS);
}

int inCostco_GasStation_Handle_Settle(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult)
{
        char    szTmp[100+1] = {0};
        
        inDISP_LogPrintf("inCostco_GasStation_Handle_AuthComplete Start");
       
        /* 加油泵號碼 */
        memcpy(srECROb->srTransDataGas.szPumpNo, gsrECR_GASStation_tb.szPumpNo,  strlen(gsrECR_GASStation_tb.szPumpNo));
        
        /* Response Code */
        switch(inCommandResult)
        {
                case VS_SUCCESS:
                        memcpy(srECROb->srTransDataGas.szResponse, "0", 1);
                        break;
                default:
                        inDISP_LogPrintf("inCommandResult = %d", inCommandResult);
                        memcpy(srECROb->srTransDataGas.szResponse, "1", 1);
                        break;
        }
        
        /* Date */
        memcpy(srECROb->srTransDataGas.szDate, pobTran->srBRec.szDate, 8);
        
        /* Time */
        memcpy(srECROb->srTransDataGas.szTime, pobTran->srBRec.szTime, 6);
        
        /* TID */
        memset(szTmp, 0x00, sizeof(szTmp));
        inGetTerminalID(szTmp);
        memcpy(srECROb->srTransDataGas.szTID, szTmp, 8);
        
        /* MID */
        memset(szTmp, 0x00, sizeof(szTmp));
        inGetMerchantID(szTmp);
        memcpy(srECROb->srTransDataGas.szMID, szTmp, 15);
        
        /* 交易總筆數 */
        inDISP_LogPrintf("inTransTotalCount = %d", srECROb->srTransDataGas.inTransTotalCount);
        /* 交易總金額 */
        inDISP_LogPrintf("inTransTotalAmount = %d", srECROb->srTransDataGas.inTransTotalAmount);                   
 
        inDISP_LogPrintf("inCostco_GasStation_Handle_AuthComplete End");
        return (VS_SUCCESS);
}

int inCostco_GasStation_Handle_PreAuth_Complete_ERR(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult)
{
        inDISP_LogPrintf("inCostco_GasStation_Handle_PreAuth_Complete_ERR Start");
       
        /* 加油泵號碼 */
        memcpy(srECROb->srTransDataGas.szPumpNo, gsrECR_GASStation_tb.szPumpNo,  strlen(gsrECR_GASStation_tb.szPumpNo));
        
        /* Response Code */
        memcpy(srECROb->srTransDataGas.szResponse, "1", 1);
        
        /* 暫時，Error String */
        if(!memcmp(gsrECR_GASStation_tb.szCommand, _Costco_szCommand_PreAuth_, 2))
        {
                memcpy(srECROb->srTransDataGas.uszErrString, "PreAuth Err", 11);
        }
        else if(!memcmp(gsrECR_GASStation_tb.szCommand, _Costco_szCommand_AuthComplete_, 2))
        {
                memcpy(srECROb->srTransDataGas.uszErrString, "Complete Err", 12);
        }
        else
        {
                inDISP_LogPrintf("Unknown (%s)", gsrECR_GASStation_tb.szCommand);
                memcpy(srECROb->srTransDataGas.uszErrString, "Other Err", 9);
        }
       
        inDISP_LogPrintf("inCostco_GasStation_Handle_PreAuth_Complete_ERR End");
        return (VS_SUCCESS);
}

/* 暫時，看不懂規格書 */
int inCostco_GasStation_Handle_ScanPay(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb, int inCommandResult)
{
        char    szTmp[100+1] = {0};
        
        inDISP_LogPrintf("inCostco_GasStation_Handle_AuthComplete Start");
       
        /* 加油泵號碼 */
        memcpy(srECROb->srTransDataGas.szPumpNo, gsrECR_GASStation_tb.szPumpNo,  strlen(gsrECR_GASStation_tb.szPumpNo));
        
        
        inDISP_LogPrintf("inCostco_GasStation_Handle_AuthComplete End");
        return (VS_SUCCESS);
}
