//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>

#include <stdio.h>

#include "usart.h"
#include "flash.h"
//--------------------------------------------------------------------------------
long sys_tick_cnt = 0;
char flag = 0;
uint8_t cnt = 0;
char buffer2[100] = { 0 };
//--------------------------------------------------------------------------------
void init_GPIO(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin	=
			GPIO_Pin_0 |
			GPIO_Pin_1 |
			GPIO_Pin_2 |
			GPIO_Pin_3 |
			GPIO_Pin_4 |
			GPIO_Pin_5 |
			GPIO_Pin_6 |
			GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
//--------------------------------------------------------------------------------
void draw(void)
{
	GPIO_Write(GPIOA, cnt);

	sprintf(buffer2, "cnt %i\n", cnt);
	USARTSend(buffer2);
}
//--------------------------------------------------------------------------------
void draw2(void)
{
	union BYTE {
		uint8_t value;
		struct {
			uint8_t bit0;
			uint8_t bit1;
			uint8_t bit2;
			uint8_t bit3;
			uint8_t bit4;
			uint8_t bit5;
			uint8_t bit6;
			uint8_t bit7;
		} bites;
	};

	union BYTE b;
	b.value = cnt;

	GPIO_WriteBit(GPIOA, GPIO_Pin_0, b.bites.bit0);
	GPIO_WriteBit(GPIOA, GPIO_Pin_1, b.bites.bit1);
	GPIO_WriteBit(GPIOA, GPIO_Pin_2, b.bites.bit2);
	GPIO_WriteBit(GPIOA, GPIO_Pin_3, b.bites.bit3);
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, b.bites.bit4);
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, b.bites.bit5);
	GPIO_WriteBit(GPIOA, GPIO_Pin_6, b.bites.bit6);
	GPIO_WriteBit(GPIOA, GPIO_Pin_7, b.bites.bit7);

	sprintf(buffer2, "cnt %i\n", cnt);
	USARTSend(buffer2);
}
//--------------------------------------------------------------------------------
void SysTick_Handler(void)
{
	sys_tick_cnt++;
	if(sys_tick_cnt > 100)	//1000
	{
		sys_tick_cnt = 0;

		flag = !flag;
		if(flag)
		{
			GPIO_SetBits(GPIOC,		GPIO_Pin_8);
			GPIO_ResetBits(GPIOC,	GPIO_Pin_9);

			draw();
			cnt++;
		}
		else
		{
			GPIO_ResetBits(GPIOC,	GPIO_Pin_8);
			GPIO_SetBits(GPIOC,		GPIO_Pin_9);
		}
	}
}
//--------------------------------------------------------------------------------
void USART1_IRQHandler(void)
{
	if ((USART1->SR & USART_FLAG_RXNE) != (u16)RESET)
	{
		RXc = USART_ReceiveData(USART1);
		RX_BUF[RXi] = RXc;
		RXi++;

		if (RXc != 13)
		{
			if (RXi > RX_BUF_SIZE-1)
			{
				clear_RXBuffer();
			}
		}
		else
		{
			RX_FLAG_END_LINE = 1;
		}

		//Echo
		USART_SendData(USART1, RXc);
	}
}
//--------------------------------------------------------------------------------
void Delay_ms(uint32_t ms)
{
	volatile uint32_t nCount;
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq (&RCC_Clocks);

	nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
	for (; nCount!=0; nCount--);
}
//--------------------------------------------------------------------------------
void vprint(const char *fmt, va_list argp)
{
	if(0 < vsprintf(buffer2, fmt, argp))
	{
		USARTSend(buffer2);
	}
}
//--------------------------------------------------------------------------------
void my_printf(const char *fmt, ...) // custom printf() function
{
	va_list argp;
	va_start(argp, fmt);
	vprint(fmt, argp);
	va_end(argp);
}
//--------------------------------------------------------------------------------
void test(void)
{
	sprintf(buffer2, "settings.Parameter5 %i\n", settings.Parameter5);
	USARTSend(buffer2);
}
//--------------------------------------------------------------------------------
int main(void)
{
	__enable_irq ();
	SysTick_Config(SystemCoreClock / 1000);
	init_GPIO();
	usart_init();

	//write_FLASH();
	read_FLASH();

	while(1)
	{
		//test();
		//my_printf("Parameter5 %i\n", settings.Parameter5);
		//Delay_ms(1000);
	}
}
//--------------------------------------------------------------------------------
