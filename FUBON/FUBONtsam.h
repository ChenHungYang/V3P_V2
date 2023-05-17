#ifndef __FUBONTSAM__
#define __FUBONTSAM__

//#include <libvoy.h>

typedef struct
{
	BYTE INS_DATA[1 + 1];
	BYTE P1_DATA[1 + 1];
	BYTE usSend_Buf[256 + 1];
	int inSendLen;
	BYTE usRece_Buf[256 + 1];
	unsigned short usReceLen;
} TFB_APDU_COMMAND;



/* ---------- [�I��]�I��[000]�@�목TSAM���O (START) ---------- */
/* �ѦҤ�� : SAM�d External Spec _2 0_ for EDC�ŧQ            */
/*[SELECT]*/
#define SAM_CLA_SELECT		        (BYTE)0x00
#define SAM_INS_SELECT			(BYTE)0xA4
#define SAM_P1_SELECT		        (BYTE)0x04
#define SAM_P2_SELECT		        (BYTE)0x00
/*------------------------------------------------------------ */
/*[CIPHER]*/
#define SAM_CLA_CIPHER		        (BYTE)0x80
#define SAM_INS_CIPHER		        (BYTE)0x44
#define SAM_P1_CIPHER		        (BYTE)0x0A
#define SAM_P2_CIPHER		        (BYTE)0x01
#define SAM_LE_CIPHER		        (BYTE)0x00
/*------------------------------------------------------------ */
/*[GEN MAC2]*/
#define SAM_CLA_GEN_MAC		        (BYTE)0x80
#define SAM_INS_GEN_MAC		        (BYTE)0x42
/* ---------- [�I��]�I��[000]�@�목TSAM���O ( END ) ---------- */


int inFUBON_TSAM_Get_Slot_Type(BYTE usSlot,DWORD *Slot_Type);
int inFUBON_TSAM_APDUTransmit(TRANSACTION_OBJECT *pobTran,DWORD Slot,BYTE *bSendBuf,int inSendLen,BYTE *bReceBuf,unsigned short *pusRespLen);
int inFUBON_TSAM_FuncInitialSLOT(TRANSACTION_OBJECT *pobTran);
int inFUBON_TSAM_InitialSLOT(DWORD Slot_Type);
int inFUBON_TSAM_EncryptISOData(TRANSACTION_OBJECT *pobTran, char *szEncrypt, int inEncryptLen);
int inFUBON_TSAM_BuildSAMAPDU(TRANSACTION_OBJECT *pobTran,
                              TFB_APDU_COMMAND *srAPDUData,
                              BYTE *bInData,
                              int inInCLen,
                              BYTE CLA,
                              BYTE Ins,
                              BYTE P1,
                              BYTE P2,
                              VS_BOOL blTCK);

int inAPDUTransmit(TRANSACTION_OBJECT *pobTran,
                       DWORD Slot,
                       BYTE *bSendBuf,
                       int inSendLen,
                       BYTE *bReceBuf,
                       unsigned short *pusRespLen);
/* ---------- [�I��]�I��[000]�@�목TSAM�[�K�ϥ� (END) ---------- */

int hex_asc_to_bcd(char *asc, unsigned char *bcd);

#endif
