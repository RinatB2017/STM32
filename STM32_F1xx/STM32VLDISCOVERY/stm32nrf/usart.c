
#include <usart.h>

struct __FILE {int handle;};
FILE __stdout;

int fputc(int ch, FILE *f)
{
	sendchar(ch);
	return 0;
}

void sendchar(int ch)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, ch);
}

void usart_init(void)
{
	USART_InitTypeDef USART_InitStructure;
	USART_StructInit(&USART_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,  ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9| GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_DeInit(USART1);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
}
void usart_put_buf (unsigned char *buf, int buf_len)
{
	unsigned int i;
	for (i=0; i<buf_len; i++)
	{
		sendchar(buf[i]);
	}
}

void usart_put_str (unsigned char *buf)
{
	unsigned int i;
	for (i=0; i<strlen(buf); i++)
	{
		sendchar(buf[i]);
	}
}
