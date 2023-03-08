#include "led.h"

//初始化PB0为输出口.并使能时钟		    
//LED IO初始化
void SYS_POWER_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOA_CLK_ENABLE(); 			//开启 GPIOA 时钟

    GPIO_Initure.Pin=GPIO_PIN_6 | GPIO_PIN_15;//PA6,PA15
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP; 			//上拉
    GPIO_Initure.Speed=GPIO_SPEED_LOW; 		//低速
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);	//PA6置1
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);//PA15置0

    __HAL_RCC_GPIOB_CLK_ENABLE(); 			//开启 GPIOB 时钟
    GPIO_Initure.Pin=GPIO_PIN_3|GPIO_PIN_4; //PB3,4
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP; 			//上拉
    GPIO_Initure.Speed=GPIO_SPEED_LOW; 		//低速
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_4,GPIO_PIN_RESET);//PB0,3,4置0
}

void SYS_POWER_ON(void)
{
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);	//PA6置0
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET); //PA15 置 1
}

void SYS_POWER_OFF(void)
{
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);	//PA6置1
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET); //PA15 置 0
}

void GPIO_CONFIG_SWD(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOA_CLK_ENABLE(); 			//开启 GPIOA 时钟
	
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_13);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_14);

	GPIO_Initure.Pin=GPIO_PIN_13;            //PA13
	GPIO_Initure.Mode=GPIO_MODE_AF_PP;      //复用推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_FAST;     //高速
	GPIO_Initure.Alternate=GPIO_AF0_SWDIO; //复用为SWDIO
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //初始化PA13

	GPIO_Initure.Pin=GPIO_PIN_14;           //PA14
	GPIO_Initure.Alternate = GPIO_AF0_SWCLK;//复用为SWCLK
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //初始化PA14
}
void ButtonTre_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOB_CLK_ENABLE();           //开启 GPIOB 时钟

    GPIO_Initure.Pin=GPIO_PIN_1;            //PB1
    GPIO_Initure.Mode=GPIO_MODE_IT_RISING;  //上升沿输入中断
    GPIO_Initure.Pull=GPIO_PULLDOWN;        //下拉

    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
    HAL_NVIC_SetPriority(EXTI0_1_IRQn,1,0);//设置中断优先级，抢占优先级1，子优先级0
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
}

void EXTI0_1_IRQHandler(void)
{
//    SystemClock_Config();
//    delay_ms(50);
//    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
//    button_state = 1;  //测试按钮响应

//    debugprint((u8*)"测试按钮响应\r\n");
    return;
}
