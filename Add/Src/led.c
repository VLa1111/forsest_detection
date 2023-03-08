#include "led.h"

//��ʼ��PB0Ϊ�����.��ʹ��ʱ��		    
//LED IO��ʼ��
void SYS_POWER_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOA_CLK_ENABLE(); 			//���� GPIOA ʱ��

    GPIO_Initure.Pin=GPIO_PIN_6 | GPIO_PIN_15;//PA6,PA15
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
    GPIO_Initure.Pull=GPIO_PULLUP; 			//����
    GPIO_Initure.Speed=GPIO_SPEED_LOW; 		//����
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);	//PA6��1
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);//PA15��0

    __HAL_RCC_GPIOB_CLK_ENABLE(); 			//���� GPIOB ʱ��
    GPIO_Initure.Pin=GPIO_PIN_3|GPIO_PIN_4; //PB3,4
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
    GPIO_Initure.Pull=GPIO_PULLUP; 			//����
    GPIO_Initure.Speed=GPIO_SPEED_LOW; 		//����
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_4,GPIO_PIN_RESET);//PB0,3,4��0
}

void SYS_POWER_ON(void)
{
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);	//PA6��0
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET); //PA15 �� 1
}

void SYS_POWER_OFF(void)
{
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);	//PA6��1
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET); //PA15 �� 0
}

void GPIO_CONFIG_SWD(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOA_CLK_ENABLE(); 			//���� GPIOA ʱ��
	
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_13);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_14);

	GPIO_Initure.Pin=GPIO_PIN_13;            //PA13
	GPIO_Initure.Mode=GPIO_MODE_AF_PP;      //�����������
	GPIO_Initure.Pull=GPIO_PULLUP;          //����
	GPIO_Initure.Speed=GPIO_SPEED_FAST;     //����
	GPIO_Initure.Alternate=GPIO_AF0_SWDIO; //����ΪSWDIO
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //��ʼ��PA13

	GPIO_Initure.Pin=GPIO_PIN_14;           //PA14
	GPIO_Initure.Alternate = GPIO_AF0_SWCLK;//����ΪSWCLK
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //��ʼ��PA14
}
void ButtonTre_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOB_CLK_ENABLE();           //���� GPIOB ʱ��

    GPIO_Initure.Pin=GPIO_PIN_1;            //PB1
    GPIO_Initure.Mode=GPIO_MODE_IT_RISING;  //�����������ж�
    GPIO_Initure.Pull=GPIO_PULLDOWN;        //����

    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
    HAL_NVIC_SetPriority(EXTI0_1_IRQn,1,0);//�����ж����ȼ�����ռ���ȼ�1�������ȼ�0
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
}

void EXTI0_1_IRQHandler(void)
{
//    SystemClock_Config();
//    delay_ms(50);
//    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
//    button_state = 1;  //���԰�ť��Ӧ

//    debugprint((u8*)"���԰�ť��Ӧ\r\n");
    return;
}
