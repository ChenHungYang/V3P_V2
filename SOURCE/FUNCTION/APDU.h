/* 
 * File:   APDU.h
 * Author: user
 *
 * Created on 2018年6月13日, 上午 9:57
 */
/* Define APDU Command */
#define _EMV_SELECT_AID_CLA_COMMAND_                   0x00
#define _EMV_SELECT_AID_INS_COMMAND_                   0xA4
#define _EMV_SELECT_AID_P1_COMMAND_                    0x04
#define _EMV_SELECT_AID_P2_COMMAND_                    0x00
#define _EMV_SELECT_AID_LC_SIZE_			 8
#define _EMV_SELECT_AID_LE_COMMAND_                    0x00

#define _FISC_SELECT_AID_CLA_COMMAND_                  0x00
#define _FISC_SELECT_AID_INS_COMMAND_                  0xA4
#define _FISC_SELECT_AID_P1_COMMAND_                    0x04
#define _FISC_SELECT_AID_P2_COMMAND_                    0x00
#define _FISC_SELECT_AID_LC_SIZE_			8
#define _FISC_SELECT_AID_LE_COMMAND_                    0x00

#define _FISC_VERIFY_PIN_CLA_COMMAND_                   0x00
#define _FISC_VERIFY_PIN_INS_COMMAND_                   0x20
#define _FISC_VERIFY_PIN_P1_COMMAND_                    0x00
#define _FISC_VERIFY_PIN_P2_COMMAND_                    0x10
#define _FISC_VERIFY_PIN_LC_SIZE_                       8

#define _FISC_SELECT_EF_CLA_COMMAND_                    0x00
#define _FISC_SELECT_EF_INS_COMMAND_                    0xA4
#define _FISC_SELECT_EF_P1_COMMAND_                     0x02
#define _FISC_SELECT_EF_P2_COMMAND_                     0x00
#define _FISC_SELECT_EF_LC_SIZE_                        2

#define _FISC_READ_RECORDS_CLA_COMMAND_                 0x00
#define _FISC_READ_RECORDS_INS_COMMAND_                 0xB2
#define _FISC_READ_RECORDS_P1_COMMAND_                  0x01
#define _FISC_READ_RECORDS_P2_COMMAND_                  0x05
#define _FISC_READ_LC_SIZE_                             0

#define _FISC_WRITE_RECORD_CLA_COMMAND_                 0x00
#define _FISC_WRITE_RECORD_INS_COMMAND_                 0xE2
#define _FISC_WRITE_RECORD_P1_COMMAND_                  0x10
#define _FISC_WRITE_RECORD_P2_COMMAND_                  0x80
#define _FISC_WRITE_RECORD_P2_COMMAND_CTLS_		0x90
#define _FISC_WRITE_RECORD_LE_COMMAND_			0x00

#define _FISC_WRITE_AUTH_RECORD_CLA_COMMAND_		0x00
#define _FISC_WRITE_AUTH_RECORD_INS_COMMAND_		0xD4
#define _FISC_WRITE_AUTH_RECORD_P1_COMMAND_		0x00
#define _FISC_WRITE_AUTH_RECORD_P2_COMMAND_		0x00
#define _FISC_WRITE_AUTH_RECORD_LE_COMMAND_		0x00

#define _FISC_READ_AUTH_RECORD_CLA_COMMAND_		0x00
#define _FISC_READ_AUTH_RECORD_INS_COMMAND_		0xB4
#define _FISC_READ_AUTH_RECORD_P1_COMMAND_		0x01
#define _FISC_READ_AUTH_RECORD_P2_COMMAND_		0x00
#define _FISC_READ_AUTH_RECORD_LE_COMMAND_		0x00

#define _WRITE_RECORD_SIZE_                             100




/* 僅紀錄用，無使用 */
/* _FISC_AID_						"\xA0\x00\x00\x01\x72\x95\x00\x01" */

typedef struct
{
        int             inSendLen;
        int             inRecevLen;
	int             inCommandDataLen;				/* DATA 欄資料長度 */
        unsigned char   uszSendData[512 + 1];
        unsigned char   uszRecevData[512 + 1];
        unsigned char   uszCommandINSData[2 + 1];			/* Instruction code */
        unsigned char   uszCommandP1Data[2 + 1];			/* Parameter 1 */
        unsigned char   uszCommandP2Data[2 + 1];			/* Parameter 2 */
        unsigned char   uszCommandData[_WRITE_RECORD_SIZE_ + 1];	/* 指令資料內容, 長度最大為Write Record Command時 */
} APDU_COMMAND;

int inAPDU_APDUTransmit(unsigned char uszSlotID, unsigned char* uszSendBuffer, unsigned short usSendLen, unsigned char* uszReceiveBuffer, unsigned short* usReceiveLen);
int inAPDU_Send_APDU_User_Slot_Process(APDU_COMMAND* srAPDU);
int inAPDU_APDUTransmit_CTLS(unsigned char* uszSendBuffer, unsigned short usSendLen, unsigned char* uszReceiveBuffer, unsigned short* usReceiveLen);
int inAPDU_Send_APDU_CTLS_Process(APDU_COMMAND* srAPDU);
int inAPDU_APDUTransmit_Flow(TRANSACTION_OBJECT *pobTran, APDU_COMMAND *srAPDU_COMMAND);
int inAPDU_BuildAPDU(APDU_COMMAND *srAPDUData);