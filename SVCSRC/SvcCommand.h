/* 
 * File:   SvcCommand.h
 * Author: Sam chang
 *
 * Created on 2022年9月13日, 下午 6:07
 */

#ifndef __SVCCOMMAND_H__
#define __SVCCOMMAND_H__

#ifdef	__cplusplus
extern "C" {
#endif


#define _COSTCO_SELECT_AID_CLA_COMMAND	0x00
#define _COSTCO_SELECT_AID_INS_COMMAND	0xA4
#define _COSTCO_SELECT_AID_P1_COMMAND		0x04
#define _COSTCO_SELECT_AID_P2_COMMAND		0x00
	
#define _COSTCO_GEN_MAC_CLA_COMMAND	0x80
#define _COSTCO_GEN_MAC_INS_COMMAND	0x40
#define _COSTCO_GEN_MAC_P1_COMMAND	0x01
#define _COSTCO_GEN_MAC_P2_COMMAND	0x01

	
int inSVC_BuildApduData(APDU_COMMAND *srAPDUData, unsigned char* CLA);
int inSVC_GenMac(TRANSACTION_OBJECT *pobTran,char* szInputData,int inInputDataLen);

#ifdef	__cplusplus
}
#endif

#endif	/* SVCCOMMAND_H */

