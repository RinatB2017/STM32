


#include "stm32f10x_rcc.h"
#include "usart.h"

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
	usart_init();

	while (1)
	{
		usart_put_str("test\n");
		delay_ms(1000);
	}
}
