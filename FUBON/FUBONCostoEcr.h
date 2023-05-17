/* 
 * File:   FUBONCostoEcr.h
 * Author: Sam chang
 *
 * Created on 2022年9月13日, 上午 11:54
 */

#ifndef FUBONCOSTOECR_H
#define	FUBONCOSTOECR_H

#ifdef	__cplusplus
extern "C" {
#endif

#define _ECR_PAGE_LOOP_0_		0	/* 選定就設成0 */
#define _ECR_PAGE_LOOP_1_		1
#define _ECR_PAGE_LOOP_2_		2
#define _ECR_PAGE_LOOP_3_		3
#define _ECR_PAGE_LOOP_4_		4
	
	
#define FU_COSTCO_ECR_107_8N1_LEN		107
#define FU_COSTCO_ECR_WAIT_TIMEOUT	120

#define _FU_COSTCO_8N1_SALE_NO_		0x21
#define _FU_COSTCO_8N1_REFUND_NO_		0x22	
	
int inFUBON_CostcoEcr_8N1_SelectSaleTransType(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inFUBON_CostcoRs232Ecr_8N1_Standard_Initial(ECR_TABLE *srECROb);
int inFUBON_CostcoRs232Ecr_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inFUBON_CostcoRs232Ecr_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inFUBON_CostcoRs232Ecr_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inFUBON_CostcoRs232Ecr_8N1_Standard_Close(ECR_TABLE* srECRob);
int inFUBON_CostcoRs232Ecr_8N1_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inFUBON_CostcoRs232Ecr_8N1_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inFUBON_CostcoRs232Ecr_8N1_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);
	
#ifdef	__cplusplus
}
#endif

#endif	/* FUBONCOSTOECR_H */

