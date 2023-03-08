#ifndef __USART_H
#define __USART_H
#include "main.h" 
#include "modbus_interface.h"
#define CurrentModeIsNBIoT	    1
#define CurrentModeIsDevice		2
#define CurrentModeIsSub		3

#define USART_REC_LEN  			128  	//����LPUART1�������ֽ��� 128
#define USART2_REC_LEN  		1024 	//����USART2�������ֽ��� 1024
#define RXBUFFERSIZE   1 //�����С
extern UART_HandleTypeDef LPUART1_Handler; //LPUART1 ���
extern UART_HandleTypeDef USART2_Handler; //LPUART1 ���
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�
extern u8 USART_RX_LEN;					//�������ݳ���
extern u8 USART_RX_flag;         		//����״̬���	
extern u32 USART_RX_Timeout;				//���ճ�ʱ����
extern u8 USART2_RX_flag;         		//����״̬���	
extern u32 USART2_RX_Timeout;				//���ճ�ʱ����
extern u8 USART2_RX_BUF[USART2_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�	
extern u16 USART2_RX_LEN;				//�������ݳ���

void UART1_Init(u32 bound);
void USART2_Init(u32 bound,u8 CurrentMode);
void MY_USART_DeInit(void);
void LPUART1_SendString(u8 *Str,u8 len);
void USART2_AT_Printf(char *str_cmd);      //str_cmd:���͵������ַ���,����Ҫ��ӻس�
  	

extern volatile u16  UartRp; //���ڶ�ȡƫ��
#define UartWp USART2_RX_LEN

void  UART1Send(u8 *send, u16 len);
void  UART2Send(u8 *send, u16 len);
u16 UART1Recv(u8*recv,u16 len);
u16 UART2Recv(u8*recv,u16 len);
void debugprint(u8 *data);
int Uart2Tunnel(int boderate, int Tunnel);
void UpBufferSend(UpBufferTemplate *roundBuff);
#endif
