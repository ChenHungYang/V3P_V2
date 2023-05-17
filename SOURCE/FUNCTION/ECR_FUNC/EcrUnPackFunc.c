#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "../../INCLUDES/Define_1.h"
#include "../../INCLUDES/Define_2.h"
#include "../../INCLUDES/TransType.h"
#include "../../INCLUDES/Transaction.h"

#include "../Sqlite.h"
#include "../HDT.h"
#include "../CFGT.h"
#include "../HDPT.h"
#include "../Function.h"

#include "ECR.h"
#include "EcrUnPackFunc.h"



int inECR_UNPACK_TimeoutResponseCode(TRANSACTION_OBJECT *pobTran, char* szUnPackBuf, int* inUnPackAddr)
{
        memcpy(szUnPackBuf, "0000", 4);
        inUnPackAddr += 4;
        
        return ECR_UNP_RL_SUCCESS;
}
