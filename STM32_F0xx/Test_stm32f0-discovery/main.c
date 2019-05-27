//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_usart.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx.h>
//--------------------------------------------------------------------------------
long sys_tick_cnt = 0;
char flag = 0;
//--------------------------------------------------------------------------------
#define RX_BUF_SIZE 80
//--------------------------------------------------------------------------------
volatile char RX_FLAG_END_LINE = 0;

volatile unsigned char RX_buffer_len = 0;
volatile char RX_buffer[RX_BUF_SIZE] = {'\0'};

volatile unsigned char TX_buffer_len = 0;
volatile char TX_buffer[RX_BUF_SIZE] = {'\0'};
//--------------------------------------------------------------------------------
void test(void);
void test2(void);
//--------------------------------------------------------------------------------
void init_GPIO(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
//--------------------------------------------------------------------------------
void usart_init(void)
{
	/* Enable USART1 and GPIOA clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9,  GPIO_AF_1); // USART1 TX
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

	/* NVIC Configuration */
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure the GPIOs */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure USART1 Tx (PA.09 & PA.10) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure the USART1 */
	USART_InitTypeDef USART_InitStructure;

	/* USART1 configuration ------------------------------------------------------*/
	/* USART1 configured as follow:
        - BaudRate = 9600 baud
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
        - USART Clock disabled
        - USART CPOL: Clock is active low
        - USART CPHA: Data is captured on the middle
        - USART LastBit: The clock pulse of the last data bit is not output to
            the SCLK pin
	 */
	USART_InitStructure.USART_BaudRate 		= 9600;
	USART_InitStructure.USART_WordLength 	= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits 		= USART_StopBits_1;
	USART_InitStructure.USART_Parity 		= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART1 */
	USART_Cmd(USART1, ENABLE);

	/* Enable the USART1 Receive interrupt: this interrupt is generated when the
        USART1 receive data register is not empty */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	NVIC_EnableIRQ (USART1_IRQn);           		// разрешить прерывания от USART1
}
//--------------------------------------------------------------------------------
void USART1_IRQHandler(void)
{
	//if ((USART1->ISR & USART_FLAG_RXNE) != (uint16_t)RESET)
	/* Прерывание по приему байта по USART */
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		USART_ClearFlag(USART1, USART_IT_RXNE);

		char s = USART_ReceiveData(USART1);
		switch(s)
		{
		case ':':
			RX_buffer_len = 0;
			break;

		case '\n':
			test2();
			RX_buffer_len = 0;
			break;

		default:
			if(RX_buffer_len < RX_BUF_SIZE)
			{
				RX_buffer[RX_buffer_len] = s;
				RX_buffer_len++;
			}
			break;
		}
	}
	if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
	{
		TX_buffer_len = 0;
	}
	USART_ClearITPendingBit(USART1, USART_IT_TC);
}
//--------------------------------------------------------------------------------
void USARTSend(const unsigned char *pucBuffer)
{
	while (*pucBuffer)
	{
		USART_SendData(USART1, *pucBuffer++);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
		{
		}
	}
}
//--------------------------------------------------------------------------------
void send_byte(uint8_t b)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET){}
	USART_SendData(USART1, b);
}
//--------------------------------------------------------------------------------
void send_data(void)
{
	int n = 0;

	send_byte((char)':');
	for(n=0; n<TX_buffer_len; n++)
	{
		send_byte(TX_buffer[n]);
	}
	send_byte((char)0x0D);
}
//--------------------------------------------------------------------------------
void test(void)
{
	TX_buffer[0] = 't';
	TX_buffer[1] = 'e';
	TX_buffer[2] = 's';
	TX_buffer[3] = 't';
	TX_buffer_len = 4;
	send_data();
}
//--------------------------------------------------------------------------------
void test2(void)
{
	TX_buffer[0] = 't';
	TX_buffer[1] = 'e';
	TX_buffer[2] = 's';
	TX_buffer[3] = 't';
	TX_buffer[4] = '2';
	TX_buffer_len = 5;
	send_data();
}
//--------------------------------------------------------------------------------
void SysTick_Handler(void)
{
	sys_tick_cnt++;
	if(sys_tick_cnt > 500)	//1000
	{
		sys_tick_cnt = 0;

		flag = !flag;
		if(flag)
		{
			GPIO_SetBits(GPIOC,		GPIO_Pin_8);
			GPIO_ResetBits(GPIOC,	GPIO_Pin_9);

			test();
		}
		else
		{
			GPIO_ResetBits(GPIOC,	GPIO_Pin_8);
			GPIO_SetBits(GPIOC,		GPIO_Pin_9);
		}
	}
}
//--------------------------------------------------------------------------------
int main(void)
{
	__enable_irq ();
	SysTick_Config(SystemCoreClock / 1000);

	init_GPIO();
	usart_init();

    while(1)
    {
    }
}
//--------------------------------------------------------------------------------
