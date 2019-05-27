#include <stdint.h>
#include <system_stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>

char Buffer_com1 [0x100];	// буффер выходящий USART1
char Buffer_com1_start [4];	// буффер USART1
long lench;	//	переменная для хранения текущего указателя буффера USART1


void init_uart(int32_t  APBCLK, int16_t BAUDRATE)
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;  //Включаем тактирование USART1

	USART1->BRR =0x341;	//57600 как и расчитанно ! х.з.8)

	USART1->CR1 |= USART_CR1_TE |USART_CR1_RE|USART_CR1_RXNEIE; // USART1
	USART1->CR1 |= USART_CR1_UE; //Включаем USART1

	NVIC_EnableIRQ(USART1_IRQn);
}

void uart1(uint8_t Data)
{
	while(!(USART1->ISR & USART_ISR_TC));
	USART1->TDR = Data;
}

void uart1_16(short data)
{
	uart1(data);
	uart1(data >> 8);
}

void uart1_32(long data)
{
	uart1(data);
	uart1(data >> 8);
	uart1(data >> 16);
	uart1(data >> 24);
}

// Обработчик прерывания USART.
void USART1_IRQHandler(void)
{
	extern char Buffer_in [];		// буффер входящий USART1
	extern short lench_Buffer_in;	// размер входящий
	long	n;

	if(USART1->ISR & USART_ISR_RXNE)
	{
		if ((Buffer_com1_start[3]==0xAA) && (Buffer_com1_start[2]==0xBB)) // преамбулa найдена
		{
			{
				Buffer_com1 [lench] = (USART1->RDR);
				lench++;// это очередной байт
				if ((Buffer_com1_start[1]+Buffer_com1_start[0]*0x100)==1) // это последний байт
				{
					for (n=0;n<lench;n++)
					{
						Buffer_in [n] = Buffer_com1 [n];
						lench_Buffer_in = lench-1;
					} // было lench
					Buffer_com1_start[3] = 0;
					lench= 0;
				}					//но после ввода crc lench-1
				if (Buffer_com1_start[1]==0)
				{
					Buffer_com1_start[0]--;
					Buffer_com1_start[1]=0xFF;
				}
				else
				{
					Buffer_com1_start[1]--;
				}
			}
		}
		else
		{
			Buffer_com1_start[3]=Buffer_com1_start[2];	// сдвиг буффера USART1
			Buffer_com1_start[2]=Buffer_com1_start[1];	// сдвиг буффера USART1
			Buffer_com1_start[1]=Buffer_com1_start[0];	// сдвиг буффера USART1
			Buffer_com1_start[0]=(USART1->RDR);
		}	// сдвиг буффера USART1
	}
}
