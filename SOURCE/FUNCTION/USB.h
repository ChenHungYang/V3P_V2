/* 
 * File:   USB.h
 * Author: user
 *
 * Created on 2017年6月26日, 下午 3:39
 */

#define _STX_					0x02
#define _ETX_					0x03
#define _ACK_					0x06
#define _NAK_					0x15
#define _ECR_USB_RETRYTIMES_			_ECR_RETRYTIMES_
#define _ECR_USB_RECEIVE_TIMER_			_ECR_RECEIVE_TIMER_
#define _ECR_USB_RECEIVE_REQUEST_TIMEOUT_	_ECR_RECEIVE_REQUEST_TIMEOUT_
#define _ECR_USB_GET_CARD_TIMEOUT_		_ECR_GET_CARD_TIMEOUT_
#define _ECR_USB_RECEIVE_ACK_TIMEOUT_		_ECR_RECEIVE_ACK_TIMEOUT_
#define _ECR_USB_BUFF_SIZE_			_ECR_BUFF_SIZE_

#define _USB_MODE_DEVICE_			d_USB_DEVICE_MODE
#define _USB_MODE_HOST_				d_USB_HOST_MODE

/* 20230413 Miyano add */
#define d_BUFF_SIZE     2048	  //Buffer Size
#define Tx_Status       0x01
#define Rx_Status       0x02
#define USB_Host        0x04
#define USB_Dev         0x08
#define USB_Notopen     0x00
#define USB_Opened      0x01

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))


int inUSB_Open(void);
int inUSB_Close(void);
int inUSB_FlushTxBuffer(void);
int inUSB_FlushRxBuffer(void);
int inUSB_Data_Send_Check(void);
int inUSB_Data_Send(unsigned char *uszSendBuff, unsigned short usSendSize);
int inUSB_Data_Receive_Check(unsigned short *usReceiveLen);
int inUSB_Data_Receive(unsigned char *uszReceBuff, unsigned short *usReceSize);

int inUSB_ECR_Initial(void);
int inUSB_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran);
int inUSB_ECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran);
int inUSB_ECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inUSB_ECR_DeInitial(void);

/* ECR介接 START */
int inUSB_ECR_Send(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize);
int inUSB_ECR_Send_ACKorNAK(ECR_TABLE * srECROb, int inAckorNak);
int inUSB_ECR_Receive_ACKandNAK(ECR_TABLE * srECROb);
int inUSB_ECR_SelectTransType(ECR_TABLE * srECROb);
int inUSB_ECR_NCCC_144_To_400(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer);
/* ECR介接 END */

/* 8N1_Standard */
int inUSB_ECR_8N1_Standard_Initial(ECR_TABLE *srECROb);
int inUSB_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUSB_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_8N1_Standard_Close(ECR_TABLE* gsrECRob);
/* 7E1_Standard*/
int inUSB_ECR_7E1_Standard_Initial(ECR_TABLE *srECROb);
int inUSB_ECR_7E1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_7E1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUSB_ECR_7E1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_7E1_Standard_Close(ECR_TABLE* gsrECRob);
/* 8N1 Customer_107_Bumper*/
int inUSB_ECR_8N1_Customer_107_Bumper_Initial(ECR_TABLE *srECROb);
int inUSB_ECR_8N1_Customer_107_Bumper_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_8N1_Customer_107_Bumper_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUSB_ECR_8N1_Customer_107_Bumper_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_8N1_Customer_107_Bumper_Close(ECR_TABLE* gsrECRob);

int inUSB_SetCDCMode(void);
int inUSB_SetSTDMode(void);
int inUSB_SetVidPid(unsigned int uiVidPid);
int inUSB_GetVidPid(unsigned int *uiVidPid);
int inUSB_HostOpen(unsigned short usVendorID, unsigned short usProductID);
int inUSB_HostClose(void);
int inUSB_HostSend(unsigned char *uszSendBuffer, unsigned long ulSendLen, unsigned long ulTransTimeout);
int inUSB_HostReceive(unsigned char *uszReceiveBuffer, unsigned long *ulReceiveLen, unsigned long ulTransTimeout);
int inUSB_SelectMode(int inMode);
int inUSB_GetStatus(unsigned int *uiStatus);
int inUSB_Get_Host_Device_Mode(int *inMode);

/* 20230413 Miyano add */
int OpenUSBPort(int USB_MODE);
int inUSB_Host_Send(unsigned char *uszSendBuff, unsigned short *usSendSize);
int inUSB_Host_Read(unsigned char *uszReadBuff, unsigned short *usReadSize);

