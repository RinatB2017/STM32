//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_flash.h>
#include "misc.h"
//--------------------------------------------------------------------------------
long sys_tick_cnt = 0;
char flag = 0;
//--------------------------------------------------------------------------------
#define RX_BUF_SIZE 80
volatile char RX_FLAG_END_LINE = 0;
volatile char RXi;
volatile char RXc;
volatile char RX_BUF[RX_BUF_SIZE] = {'\0'};
volatile char buffer[80] = {'\0'};
//--------------------------------------------------------------------------------
void init_GPIO(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
//--------------------------------------------------------------------------------
void SysTick_Handler(void)
{
	sys_tick_cnt++;
	if(sys_tick_cnt > 1000)
	{
		sys_tick_cnt = 0;

		flag = !flag;
		if(flag)
		{
			GPIO_SetBits(GPIOC,		GPIO_Pin_8);
			GPIO_ResetBits(GPIOC,	GPIO_Pin_9);
		}
		else
		{
			GPIO_ResetBits(GPIOC,	GPIO_Pin_8);
			GPIO_SetBits(GPIOC,		GPIO_Pin_9);
		}
	}
}
//--------------------------------------------------------------------------------
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
        - BaudRate = 115200 baud
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
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);

    /* Enable USART1 */
    USART_Cmd(USART1, ENABLE);

    /* Enable the USART1 Receive interrupt: this interrupt is generated when the
        USART1 receive data register is not empty */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}
//--------------------------------------------------------------------------------
void USART1_IRQHandler(void)
{
    if ((USART1->SR & USART_FLAG_RXNE) != (u16)RESET)
    {
            RXc = USART_ReceiveData(USART1);
            RX_BUF[RXi] = RXc;
            RXi++;

            if (RXc != 13) {
                if (RXi > RX_BUF_SIZE-1) {
                    clear_RXBuffer();
                }
            }
            else {
                RX_FLAG_END_LINE = 1;
            }

            //Echo
            USART_SendData(USART1, RXc);
    }
}
//--------------------------------------------------------------------------------
void clear_RXBuffer(void)
{
    for (RXi=0; RXi<RX_BUF_SIZE; RXi++)
        RX_BUF[RXi] = '\0';
    RXi = 0;
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
void Delay_ms(uint32_t ms)
{
	volatile uint32_t nCount;
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq (&RCC_Clocks);

	nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
	for (; nCount!=0; nCount--);
}
//--------------------------------------------------------------------------------
#define MY_FLASH_PAGE_ADDR 0x800FC00

typedef struct
  {
    char Parameter1;        // 1 byte
    uint8_t Parameter2;     // 1 byte
    uint16_t Parameter3;    // 2 byte
    uint32_t Parameter4;    // 4 byte
    uint32_t Parameter5;    // 4 byte

                            // 8 byte = 2  32-bits words.  It's - OK
                            // !!! Full size (bytes) must be a multiple of 4 !!!
  } tpSettings;
tpSettings settings;

#define SETTINGS_WORDS sizeof(settings)/4
//--------------------------------------------------------------------------------
void FLASH_Init(void)
{
    /* Next commands may be used in SysClock initialization function
       In this case using of FLASH_Init is not obligatorily */
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd( FLASH_PrefetchBuffer_Enable);
    /* Flash 2 wait state */
    FLASH_SetLatency( FLASH_Latency_2);
}
//--------------------------------------------------------------------------------
void FLASH_ReadSettings(void)
{
    //Read settings
    uint32_t *source_addr = (uint32_t *)MY_FLASH_PAGE_ADDR;
    uint32_t *dest_addr = (void *)&settings;
    for (uint16_t i=0; i<SETTINGS_WORDS; i++) {
        *dest_addr = *(__IO uint32_t*)source_addr;
        source_addr++;
        dest_addr++;
    }
}
//--------------------------------------------------------------------------------
void FLASH_WriteSettings(void)
{
    FLASH_Unlock();
    FLASH_ErasePage(MY_FLASH_PAGE_ADDR);

    // Write settings
    uint32_t *source_addr = (void *)&settings;
    uint32_t *dest_addr = (uint32_t *) MY_FLASH_PAGE_ADDR;
    for (uint16_t i=0; i<SETTINGS_WORDS; i++) {
        FLASH_ProgramWord((uint32_t)dest_addr, *source_addr);
        source_addr++;
        dest_addr++;
    }

    FLASH_Lock();
}
//--------------------------------------------------------------------------------
int main(void)
{
	__enable_irq ();
	SysTick_Config(SystemCoreClock / 1000);
	init_GPIO();
	usart_init();

#if 0
    FLASH_Init();
    FLASH_ReadSettings();

    settings.Parameter1 = 1;
    settings.Parameter2 = 2;
    settings.Parameter3 = 3;
    settings.Parameter4 = 4;

    settings.Parameter5 = 1000000;

    FLASH_WriteSettings();
#endif

    FLASH_Init();
    FLASH_ReadSettings();

    while(1)
    {
    	USARTSend("test\n");
    	Delay_ms(1000);
    }
}
//--------------------------------------------------------------------------------
