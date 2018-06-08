#include "stm32f10x_RCC.h"
#include "stm32f10x_GPIO.h"
#include "nRF24l01P.h"
#include <misc.h>
#include "usart.h"

int main(void)
{
	SystemInit();

	usart_init();

	// usart_put_str("Init usart\n");

	nRF24L01_HW_Init();

	// TX_Mode();
	RX_Mode();

	while(1)
	{
		while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)!=0); // ожидание готовности данных приемника прерывание от IRQ
		// usart_put_str("found\n");
		nRF24L01_RxPacket();
	}
}

