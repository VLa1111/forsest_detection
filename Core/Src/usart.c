#include "usart.h"
#include "main.h"
#include "string.h"
#include "stdio.h"
#include "modbus_interface.h"
#if 1
#pragma import(__use_no_semihosting)
struct __FILE       //标准库需要的支持函数
{
    int handle;
};
FILE __stdout;
void _sys_exit(int x) //定义_sys_exit()以避免使用半主机模式
{
    x = x;
}
int fputc(int ch, FILE *f)      //重定义fputc函数
{
    while((LPUART1->ISR&0X40)==0);//循环发送,直到发送完毕
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


u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节
u8 USART_RX_LEN=0;                  //接收数据长度
u8 USART2_RX_BUF[USART2_REC_LEN];   //接收缓冲,最大USART2_REC_LEN个字节
u16 USART2_RX_LEN=0;                //接收数据长度
u8 USART_RX_flag=0;                 //接收状态标记
u32 USART_RX_Timeout=0;             //接收超时计数
u8 USART2_RX_flag=0;                //接收状态标记
u32 USART2_RX_Timeout=0;            //接收超时计数
u8 aRxBuffer[RXBUFFERSIZE];         //HAL库使用的串口接收缓冲

volatile u16  UartRp; //串口读取偏移

UART_HandleTypeDef LPUART1_Handler; //LPUART1 句柄
UART_HandleTypeDef USART2_Handler; //USART2 句柄

//初始化IO 低功耗串口1
//bound:波特率
void UART1_Init(u32 bound)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_Initure;
    //UART 初始化设置
    LPUART1_Handler.Instance=LPUART1;                       //LPUART1
    LPUART1_Handler.Init.BaudRate=bound;                    //波特率
    LPUART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;     //字长为8位数据格式
    LPUART1_Handler.Init.StopBits=UART_STOPBITS_1;          //一个停止位
    LPUART1_Handler.Init.Parity=UART_PARITY_NONE;           //无奇偶校验位
    LPUART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;     //无硬件流控
    LPUART1_Handler.Init.Mode=UART_MODE_TX_RX;              //收发模式

    __HAL_RCC_GPIOA_CLK_ENABLE();           //使能GPIOA时钟
    __HAL_RCC_LPUART1_CLK_ENABLE();         //使能LPUART1时钟
    __HAL_UART_ENABLE(&LPUART1_Handler);    //使能句柄 handler 指定的串口

    GPIO_Initure.Pin=GPIO_PIN_14;           //PA14
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;      //复用推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FAST;     //高速
    GPIO_Initure.Alternate=GPIO_AF6_LPUART1;//复用为LPUART1
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //初始化PA14

    GPIO_Initure.Pin=GPIO_PIN_13;           //PA13
    GPIO_Initure.Alternate = GPIO_AF6_LPUART1;//复用为LPUART1
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //初始化PA13

    HAL_NVIC_EnableIRQ(LPUART1_IRQn);       //使能USART1中断通道
    HAL_NVIC_SetPriority(LPUART1_IRQn,1,5); //抢占优先级1，子优先级1

    HAL_UART_Init(&LPUART1_Handler);                                        //HAL_UART_Init()会使能LPUART1

    __HAL_UART_ENABLE_IT(&LPUART1_Handler,UART_IT_RXNE);  //开启接收完成中断
    //HAL_UART_Receive_IT(&LPUART1_Handler,(u8 *)aRxBuffer,RXBUFFERSIZE);//该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
}

//初始化IO 串口2
//bound:波特率
void USART2_Init(u32 bound,u8 CurrentMode)
{
    //GPIO端口设置
    //HAL_UART_DeInit(&USART2_Handler);
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_UART_DISABLE_IT(&USART2_Handler,UART_IT_RXNE);  //关闭接收完成中断
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_USART2_CLK_DISABLE();         //失能USART2时钟
    __HAL_UART_DISABLE(&USART2_Handler);    //失能句柄 handler 指定的串口
    __HAL_RCC_USART2_RELEASE_RESET();
    __HAL_RCC_USART2_RELEASE_RESET();
    //Disable the NVIC for UART
    HAL_NVIC_DisableIRQ(USART2_IRQn);


    //UART 初始化设置
    USART2_Handler.Instance=USART2;                     //USART2
    USART2_Handler.Init.BaudRate=bound;                 //波特率
    USART2_Handler.Init.WordLength=UART_WORDLENGTH_8B;      //字长为8位数据格式
    USART2_Handler.Init.StopBits=UART_STOPBITS_1;           //一个停止位
    USART2_Handler.Init.Parity=UART_PARITY_NONE;            //无奇偶校验位
    USART2_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;      //无硬件流控
    USART2_Handler.Init.Mode=UART_MODE_TX_RX;               //收发模式

    __HAL_RCC_GPIOA_CLK_ENABLE();           //使能GPIOA时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();           //使能GPIOA时钟
    __HAL_RCC_USART2_CLK_ENABLE();          //使能USART2时钟
    __HAL_UART_ENABLE(&USART2_Handler);     //使能句柄 handler 指定的串口

    if(CurrentMode==CurrentModeIsNBIoT)
    {
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);

        GPIO_Initure.Pin=GPIO_PIN_2;            //PA2
        GPIO_Initure.Mode=GPIO_MODE_AF_PP;      //复用推挽输出
        GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
        GPIO_Initure.Speed=GPIO_SPEED_FAST;     //高速
        GPIO_Initure.Alternate=GPIO_AF4_USART2; //复用为USART2
        HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //初始化PA2

        GPIO_Initure.Pin=GPIO_PIN_3;            //PA3
        GPIO_Initure.Alternate = GPIO_AF4_USART2;//复用为USART2
        HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //初始化PA3
    }
    else if(CurrentMode==CurrentModeIsDevice)
    {
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);

        GPIO_Initure.Pin=GPIO_PIN_6;            //PB6
        GPIO_Initure.Mode=GPIO_MODE_AF_PP;      //复用推挽输出
        GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
        GPIO_Initure.Speed=GPIO_SPEED_FAST;     //高速
        GPIO_Initure.Alternate=GPIO_AF0_USART2; //复用为USART2
        HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //初始化PB6

        GPIO_Initure.Pin=GPIO_PIN_7;            //PB7
        GPIO_Initure.Alternate = GPIO_AF0_USART2;//复用为USART2
        HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //初始化PB7
    }
    else if(CurrentMode==CurrentModeIsSub)
    {
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

        GPIO_Initure.Pin=GPIO_PIN_9;            //PA9
        GPIO_Initure.Mode=GPIO_MODE_AF_PP;      //复用推挽输出
        GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
        GPIO_Initure.Speed=GPIO_SPEED_FAST;     //高速
        GPIO_Initure.Alternate=GPIO_AF4_USART2; //复用为USART2
        HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //初始化PA9

        GPIO_Initure.Pin=GPIO_PIN_10;           //PA10
        GPIO_Initure.Alternate = GPIO_AF4_USART2;//复用为USART2
        HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //初始化PA10
    }
    HAL_NVIC_EnableIRQ(USART2_IRQn);        //使能USART1中断通道
    HAL_NVIC_SetPriority(USART2_IRQn,1,0);  //抢占优先级1，子优先级0

    HAL_UART_Init(&USART2_Handler);                                     //HAL_UART_Init()会使能LPUART1

    __HAL_UART_ENABLE_IT(&USART2_Handler,UART_IT_RXNE);  //开启接收完成中断
    //HAL_UART_Receive_IT(&LPUART1_Handler,(u8 *)aRxBuffer,RXBUFFERSIZE);//该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
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
    __HAL_RCC_LPUART1_CLK_DISABLE();        //失能LPUART1时钟
	__HAL_RCC_USART2_CLK_DISABLE();         //失能USART2时钟
	
	__HAL_UART_DISABLE(&LPUART1_Handler);    //失能句柄 handler 指定的串口
    __HAL_RCC_LPUART1_RELEASE_RESET();

    __HAL_UART_DISABLE(&USART2_Handler);    //失能句柄 handler 指定的串口
    __HAL_RCC_USART2_RELEASE_RESET();
}


//低功耗串口1中断服务程序
void LPUART1_IRQHandler(void)
{
    if((__HAL_UART_GET_FLAG(&LPUART1_Handler,UART_FLAG_RXNE)!=RESET))//接收中断
    {
        USART_RX_BUF[USART_RX_LEN]=LPUART1->RDR;
        USART_RX_LEN++;
        USART_RX_flag=1;
        USART_RX_Timeout=0;
		if(USART_RX_LEN >=USART_REC_LEN )USART_RX_LEN = 0;
    }
    HAL_UART_IRQHandler(&LPUART1_Handler);
}


//串口2中断服务程序
void USART2_IRQHandler(void)
{	
    if((__HAL_UART_GET_FLAG(&USART2_Handler,UART_FLAG_RXNE)!=RESET))//接收中断
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
