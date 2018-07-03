//------------------------------------------------------------
//
//------------------------------------------------------------
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "string.h"
//------------------------------------------------------------
#include "matrix.h"
//------------------------------------------------------------
#define RX_BUF_SIZE 80
volatile char RX_FLAG_END_LINE = 0;
volatile char RXi;  // индекс принятых данных (количество)
volatile char RXc;  // текущий принятый символ
volatile char RX_BUF[RX_BUF_SIZE] = {'\0'};
//------------------------------------------------------------
#pragma pack (push, 1)

typedef struct HEADER
{
  uint8_t   addr;   // адрес модуля
  uint8_t   cmd;    // команда
  uint16_t  len;    // длина данных
  uint8_t   data[]; // данные
} header_t;

#pragma pack(pop)
//------------------------------------------------------------
uint16_t crc16(uint8_t *pcBlock, uint8_t len)
{
  uint16_t a, crc = 0xFFFF;
  while (len--)
  {
    a = *pcBlock++ << 8;
    crc ^= a;
    for (int i = 0; i < 8; i++)
      crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
  }
  return crc;
}
//------------------------------------------------------------
void clear_RXBuffer(void)
{
	for (RXi=0; RXi<RX_BUF_SIZE; RXi++)
		RX_BUF[RXi] = '\0';
	RXi = 0;
}
//------------------------------------------------------------
void usart_init(void)
{
	/* Enable USART1 and GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	/* NVIC Configuration */
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure the GPIOs */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure the USART1 */
	USART_InitTypeDef USART_InitStructure;

	/* USART1 configuration ------------------------------------------------------*/
	/* USART1 configured as follow:
		- BaudRate = 57600 baud
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
	USART_InitStructure.USART_BaudRate = 57600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART1 */
	USART_Cmd(USART1, ENABLE);

	/* 
         * Enable the USART1 Receive interrupt: this interrupt is generated when the
         * USART1 receive data register is not empty 
         */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}
//------------------------------------------------------------
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
		//USART_SendData(USART1, RXc);
	}
}
//------------------------------------------------------------
void USARTSend(char *pucBuffer)
{
	while (*pucBuffer)
	{
		USART_SendData(USART1, *pucBuffer++);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
		{
		}
	}
}
//------------------------------------------------------------
void Delay_ms(uint32_t ms)
{
	volatile uint32_t nCount;
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq (&RCC_Clocks);

	nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
	for (; nCount!=0; nCount--);
}
//------------------------------------------------------------
HSV_t hsv_buffer[DEBUG_SIZE];
int main(void)
{
	ws2812b_Init();

	// Initialize USART
	usart_init();
	USARTSend(" USART1 is ready.\n");

	init_addr_matrix();

	//clear_leds();
	while (!ws2812b_IsReady()); // wait
        
        //------------------------------------------
#if 1
        //TODO test
        HSV_t led;
        led.h = 0;      // тон          0..360
        led.s = 100;    // насыщенность 0..100
        led.v = 100     // яркость      0..100
        
        for(int n=0; n<DEBUG_SIZE; n++;
        {
            hsv_buffer[n] = led;
        }
        ws2812b_SendHSV(hsv_buffer, DEBUG_SIZE);
        while(1)
        {
        }
#endif
        //------------------------------------------
	
        ws2812b_SendRGB(led_buffer, DEBUG_SIZE);

	led_index = 0;
	while (1)
	{
		if (RX_FLAG_END_LINE == 1)
		{
			// Reset END_LINE Flag
			RX_FLAG_END_LINE = 0;

			if (strncmp(strupr(RX_BUF), "+\r", 2) == 0)
			{
				if(led_index < MAX_SIZE)
				{
					led_index ++;
				}
				while (!ws2812b_IsReady()); // wait
				ws2812b_SendRGB(&led_buffer[led_index], DEBUG_SIZE);
				//USARTSend("+\n");
			}
			if (strncmp(strupr(RX_BUF), "-\r", 2) == 0)
			{
				if(led_index > 0)
				{
					led_index --;
				}
				while (!ws2812b_IsReady()); // wait
				ws2812b_SendRGB(&led_buffer[led_index], DEBUG_SIZE);
				//USARTSend("+\n");
			}
			clear_RXBuffer();
		}
	}
}
//------------------------------------------------------------
