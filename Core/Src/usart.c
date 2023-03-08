#include "usart.h"
#include "main.h"
#include "string.h"
#include "stdio.h"
#include "modbus_interface.h"
#if 1
#pragma import(__use_no_semihosting)
struct __FILE       //��׼����Ҫ��֧�ֺ���
{
    int handle;
};
FILE __stdout;
void _sys_exit(int x) //����_sys_exit()�Ա���ʹ�ð�����ģʽ
{
    x = x;
}
int fputc(int ch, FILE *f)      //�ض���fputc����
{
    while((LPUART1->ISR&0X40)==0);//ѭ������,ֱ���������
    LPUART1->TDR = (u8) ch;
    return ch;
}

void printbuffer(u8* buffer,u16 len)
{
    u16 i;
    //if(terInfo.log == needLog)
    {
        for(i=0; i<len; i++)
        {
            printf("%02x",buffer[i]);
        }
    }
}

void debugprint(u8 *data)
{
    //if(terInfo.log == needLog)
    {
        printf((char *)data);
    }
}
#endif


u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�
u8 USART_RX_LEN=0;                  //�������ݳ���
u8 USART2_RX_BUF[USART2_REC_LEN];   //���ջ���,���USART2_REC_LEN���ֽ�
u16 USART2_RX_LEN=0;                //�������ݳ���
u8 USART_RX_flag=0;                 //����״̬���
u32 USART_RX_Timeout=0;             //���ճ�ʱ����
u8 USART2_RX_flag=0;                //����״̬���
u32 USART2_RX_Timeout=0;            //���ճ�ʱ����
u8 aRxBuffer[RXBUFFERSIZE];         //HAL��ʹ�õĴ��ڽ��ջ���

volatile u16  UartRp; //���ڶ�ȡƫ��

UART_HandleTypeDef LPUART1_Handler; //LPUART1 ���
UART_HandleTypeDef USART2_Handler; //USART2 ���

//��ʼ��IO �͹��Ĵ���1
//bound:������
void UART1_Init(u32 bound)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_Initure;
    //UART ��ʼ������
    LPUART1_Handler.Instance=LPUART1;                       //LPUART1
    LPUART1_Handler.Init.BaudRate=bound;                    //������
    LPUART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;     //�ֳ�Ϊ8λ���ݸ�ʽ
    LPUART1_Handler.Init.StopBits=UART_STOPBITS_1;          //һ��ֹͣλ
    LPUART1_Handler.Init.Parity=UART_PARITY_NONE;           //����żУ��λ
    LPUART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;     //��Ӳ������
    LPUART1_Handler.Init.Mode=UART_MODE_TX_RX;              //�շ�ģʽ

    __HAL_RCC_GPIOA_CLK_ENABLE();           //ʹ��GPIOAʱ��
    __HAL_RCC_LPUART1_CLK_ENABLE();         //ʹ��LPUART1ʱ��
    __HAL_UART_ENABLE(&LPUART1_Handler);    //ʹ�ܾ�� handler ָ���Ĵ���

    GPIO_Initure.Pin=GPIO_PIN_14;           //PA14
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;      //�����������
    GPIO_Initure.Pull=GPIO_PULLUP;          //����
    GPIO_Initure.Speed=GPIO_SPEED_FAST;     //����
    GPIO_Initure.Alternate=GPIO_AF6_LPUART1;//����ΪLPUART1
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //��ʼ��PA14

    GPIO_Initure.Pin=GPIO_PIN_13;           //PA13
    GPIO_Initure.Alternate = GPIO_AF6_LPUART1;//����ΪLPUART1
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //��ʼ��PA13

    HAL_NVIC_EnableIRQ(LPUART1_IRQn);       //ʹ��USART1�ж�ͨ��
    HAL_NVIC_SetPriority(LPUART1_IRQn,1,5); //��ռ���ȼ�1�������ȼ�1

    HAL_UART_Init(&LPUART1_Handler);                                        //HAL_UART_Init()��ʹ��LPUART1

    __HAL_UART_ENABLE_IT(&LPUART1_Handler,UART_IT_RXNE);  //������������ж�
    //HAL_UART_Receive_IT(&LPUART1_Handler,(u8 *)aRxBuffer,RXBUFFERSIZE);//�ú����Ὺ�������жϣ���־λUART_IT_RXNE���������ý��ջ����Լ����ջ���������������
}

//��ʼ��IO ����2
//bound:������
void USART2_Init(u32 bound,u8 CurrentMode)
{
    //GPIO�˿�����
    //HAL_UART_DeInit(&USART2_Handler);
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_UART_DISABLE_IT(&USART2_Handler,UART_IT_RXNE);  //�رս�������ж�
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_USART2_CLK_DISABLE();         //ʧ��USART2ʱ��
    __HAL_UART_DISABLE(&USART2_Handler);    //ʧ�ܾ�� handler ָ���Ĵ���
    __HAL_RCC_USART2_RELEASE_RESET();
    __HAL_RCC_USART2_RELEASE_RESET();
    //Disable the NVIC for UART
    HAL_NVIC_DisableIRQ(USART2_IRQn);


    //UART ��ʼ������
    USART2_Handler.Instance=USART2;                     //USART2
    USART2_Handler.Init.BaudRate=bound;                 //������
    USART2_Handler.Init.WordLength=UART_WORDLENGTH_8B;      //�ֳ�Ϊ8λ���ݸ�ʽ
    USART2_Handler.Init.StopBits=UART_STOPBITS_1;           //һ��ֹͣλ
    USART2_Handler.Init.Parity=UART_PARITY_NONE;            //����żУ��λ
    USART2_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;      //��Ӳ������
    USART2_Handler.Init.Mode=UART_MODE_TX_RX;               //�շ�ģʽ

    __HAL_RCC_GPIOA_CLK_ENABLE();           //ʹ��GPIOAʱ��
    __HAL_RCC_GPIOB_CLK_ENABLE();           //ʹ��GPIOAʱ��
    __HAL_RCC_USART2_CLK_ENABLE();          //ʹ��USART2ʱ��
    __HAL_UART_ENABLE(&USART2_Handler);     //ʹ�ܾ�� handler ָ���Ĵ���

    if(CurrentMode==CurrentModeIsNBIoT)
    {
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);

        GPIO_Initure.Pin=GPIO_PIN_2;            //PA2
        GPIO_Initure.Mode=GPIO_MODE_AF_PP;      //�����������
        GPIO_Initure.Pull=GPIO_PULLUP;          //����
        GPIO_Initure.Speed=GPIO_SPEED_FAST;     //����
        GPIO_Initure.Alternate=GPIO_AF4_USART2; //����ΪUSART2
        HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //��ʼ��PA2

        GPIO_Initure.Pin=GPIO_PIN_3;            //PA3
        GPIO_Initure.Alternate = GPIO_AF4_USART2;//����ΪUSART2
        HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //��ʼ��PA3
    }
    else if(CurrentMode==CurrentModeIsDevice)
    {
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);

        GPIO_Initure.Pin=GPIO_PIN_6;            //PB6
        GPIO_Initure.Mode=GPIO_MODE_AF_PP;      //�����������
        GPIO_Initure.Pull=GPIO_PULLUP;          //����
        GPIO_Initure.Speed=GPIO_SPEED_FAST;     //����
        GPIO_Initure.Alternate=GPIO_AF0_USART2; //����ΪUSART2
        HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //��ʼ��PB6

        GPIO_Initure.Pin=GPIO_PIN_7;            //PB7
        GPIO_Initure.Alternate = GPIO_AF0_USART2;//����ΪUSART2
        HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //��ʼ��PB7
    }
    else if(CurrentMode==CurrentModeIsSub)
    {
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

        GPIO_Initure.Pin=GPIO_PIN_9;            //PA9
        GPIO_Initure.Mode=GPIO_MODE_AF_PP;      //�����������
        GPIO_Initure.Pull=GPIO_PULLUP;          //����
        GPIO_Initure.Speed=GPIO_SPEED_FAST;     //����
        GPIO_Initure.Alternate=GPIO_AF4_USART2; //����ΪUSART2
        HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //��ʼ��PA9

        GPIO_Initure.Pin=GPIO_PIN_10;           //PA10
        GPIO_Initure.Alternate = GPIO_AF4_USART2;//����ΪUSART2
        HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //��ʼ��PA10
    }
    HAL_NVIC_EnableIRQ(USART2_IRQn);        //ʹ��USART1�ж�ͨ��
    HAL_NVIC_SetPriority(USART2_IRQn,1,0);  //��ռ���ȼ�1�������ȼ�0

    HAL_UART_Init(&USART2_Handler);                                     //HAL_UART_Init()��ʹ��LPUART1

    __HAL_UART_ENABLE_IT(&USART2_Handler,UART_IT_RXNE);  //������������ж�
    //HAL_UART_Receive_IT(&LPUART1_Handler,(u8 *)aRxBuffer,RXBUFFERSIZE);//�ú����Ὺ�������жϣ���־λUART_IT_RXNE���������ý��ջ����Լ����ջ���������������
}

void MY_USART_DeInit(void)
{
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_13);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_14);
    __HAL_RCC_LPUART1_CLK_DISABLE();        //ʧ��LPUART1ʱ��
	__HAL_RCC_USART2_CLK_DISABLE();         //ʧ��USART2ʱ��
	
	__HAL_UART_DISABLE(&LPUART1_Handler);    //ʧ�ܾ�� handler ָ���Ĵ���
    __HAL_RCC_LPUART1_RELEASE_RESET();

    __HAL_UART_DISABLE(&USART2_Handler);    //ʧ�ܾ�� handler ָ���Ĵ���
    __HAL_RCC_USART2_RELEASE_RESET();
}


//�͹��Ĵ���1�жϷ������
void LPUART1_IRQHandler(void)
{
    if((__HAL_UART_GET_FLAG(&LPUART1_Handler,UART_FLAG_RXNE)!=RESET))//�����ж�
    {
        USART_RX_BUF[USART_RX_LEN]=LPUART1->RDR;
        USART_RX_LEN++;
        USART_RX_flag=1;
        USART_RX_Timeout=0;
		if(USART_RX_LEN >=USART_REC_LEN )USART_RX_LEN = 0;
    }
    HAL_UART_IRQHandler(&LPUART1_Handler);
}


//����2�жϷ������
void USART2_IRQHandler(void)
{	
    if((__HAL_UART_GET_FLAG(&USART2_Handler,UART_FLAG_RXNE)!=RESET))//�����ж�
    {
        USART2_RX_BUF[USART2_RX_LEN]=USART2->RDR;
        USART2_RX_LEN++;
		if(USART2_RX_LEN >=USART2_REC_LEN )USART2_RX_LEN = 0;
    }
    HAL_UART_IRQHandler(&USART2_Handler);
}

u16 UART1Recv(u8*recv,u16 len)
{
		if(USART_RX_LEN > 0 && USART_RX_LEN <= len)
		{
			memcpy(recv,USART_RX_BUF,USART_RX_LEN);
		}
		else
		{
			memcpy(recv,USART_RX_BUF,len);
		}
		memset(USART_RX_BUF,0,USART_REC_LEN);
		USART_RX_LEN = 0;
		return 0;
}

u16 UART2Recv(u8*recv,u16 len)
{
		if(USART2_RX_LEN > 0 && USART2_RX_LEN <= len)
		{
			memcpy(recv,USART2_RX_BUF,USART2_RX_LEN);
		}
		else
		{
			memcpy(recv,USART2_RX_BUF,len);
		}
		memset(USART2_RX_BUF,0,USART2_REC_LEN);
		USART2_RX_LEN = 0;	
		return 0;		
	//HAL_UART_Receive_IT(&USART2_Handler, recv, len);
}

void  UART1Send(u8 *send, u16 len)
{
    HAL_UART_Transmit(&LPUART1_Handler,send,len,1000);
}

void  UART2Send(u8 *send, u16 len)
{
    HAL_UART_Transmit(&USART2_Handler,send,len,1000);
}

int Uart2Tunnel(int boderate, int Tunnel)
{
  if (Tunnel == CurrentModeIsNBIoT){
      USART2_Init(boderate, CurrentModeIsNBIoT);
			return 1;
  }
  else if (Tunnel == CurrentModeIsDevice){
      USART2_Init(boderate, CurrentModeIsDevice);
			return 1;
  }
  else if (Tunnel == CurrentModeIsSub){
      USART2_Init(boderate, CurrentModeIsSub);
			return 1;
  }
	else return 0;
}

void UpBufferSend(UpBufferTemplate *roundBuff)
{
    u8 tempBuff[20];
    memcpy(tempBuff,     roundBuff->CONum,  4);
    memcpy(&tempBuff[4], roundBuff->WindDir,4);
    memcpy(&tempBuff[8], roundBuff->WindSp, 4);
    memcpy(&tempBuff[12],roundBuff->Temp,   4);
    memcpy(&tempBuff[16],roundBuff->Moi,    4);

    UART2Send(tempBuff,20);
}
