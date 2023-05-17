/* 
 * File:   RS232.h
 * Author: carolyn
 *
 * Created on 2016年1月6日, 上午 9:34
 */

#define _STX_					0x02
#define _ETX_					0x03
#define _ACK_					0x06
#define _NAK_					0x15
#define _ECR_RS232_RETRYTIMES_				_ECR_RETRYTIMES_
#define _ECR_RS232_RECEIVE_TIMER_			_ECR_RECEIVE_TIMER_
#define _ECR_RS232_RECEIVE_REQUEST_TIMEOUT_	_ECR_RECEIVE_REQUEST_TIMEOUT_
#define _ECR_RS232_GET_CARD_TIMEOUT_		_ECR_GET_CARD_TIMEOUT_
#define _ECR_RS232_RECEIVE_ACK_TIMEOUT_		_ECR_RECEIVE_ACK_TIMEOUT_
#define _ECR_RS232_BUFF_SIZE_				_ECR_BUFF_SIZE_

/* RS232基本功能 */
int inRS232_Open(unsigned char uszComport, unsigned long ulBaudRate, unsigned char uszParity, unsigned char uszDataBits, unsigned char uszStopBits);
int inRS232_Close(unsigned char uszComport);
int inRS232_FlushRxBuffer(unsigned char uszComPort);
int inRS232_FlushTxBuffer(unsigned char uszComPort);
int inRS232_Data_Send_Check(unsigned char uszComPort);
int inRS232_Data_Send(unsigned char uszComPort, unsigned char *uszReceBuff, unsigned short usReceSize);
int inRS232_Data_Receive_Check(unsigned char uszComPort, unsigned short *usReceiveLen);
int inRS232_Data_Receive(unsigned char uszComPort, unsigned char *uszReceBuff, unsigned short *usReceSize);

/* 這裡用的是global變數 gsrECROb START */
int inRS232_ECR_Initial(void);
int inRS232_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran);
int inRS232_ECR_Load_TMK_From_520(TRANSACTION_OBJECT *pobTran);
int inRS232_ECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran);
int inRS232_ECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inRS232_ECR_DeInitial(void);
/* 這裡用的是global變數 gsrECROb END */

/* ECR介接 START */
int inRS232_ECR_NCCC_7E1_To_8N1(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer);
int inRS232_ECR_NCCC_7E1_To_8N1_Cheat(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer);
int inRS232_ECR_SelectTransType(ECR_TABLE * srECROb);
int inRS232_ECR_SelectOtherCardType(TRANSACTION_OBJECT *pobTran);
/* ECR介接 END */

/* 8N1_Standard */
int inRS232_ECR_8N1_Standard_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_8N1_Standard_Close(ECR_TABLE* gsrECRob);
/* 7E1_Standard*/
int inRS232_ECR_7E1_Standard_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_7E1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_7E1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_7E1_Standard_Close(ECR_TABLE* gsrECRob);
/* LOAD_TMK_FROM_520_Standard */
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Close(ECR_TABLE* gsrECRob);

/* 8N1_Customer_107_Bumper */
int inRS232_ECR_8N1_Customer_107_Bumper_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_8N1_Customer_107_Bumper_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_8N1_Customer_107_Bumper_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Customer_107_Bumper_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_8N1_Customer_107_Bumper_Close(ECR_TABLE* gsrECRob);
