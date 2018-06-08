
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

void delay_ms(uint32_t ms)
{
	volatile uint32_t nCount;
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq (&RCC_Clocks);

	nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
	for (; nCount!=0; nCount--);
}

int main(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	while(1)
	{
		GPIO_SetBits(GPIOB,	GPIO_Pin_12);
		delay_ms(500);
		GPIO_ResetBits(GPIOB,	GPIO_Pin_12);
		delay_ms(500);
	}
}
