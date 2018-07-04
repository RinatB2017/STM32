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
#include "ws2812b.h"
#include "matrix.h"
//------------------------------------------------------------
#define RX_BUF_SIZE 100
volatile char RX_FLAG_END_LINE = 0;
volatile char RXi;  // индекс прин€тых данных (количество)
volatile char RXc;  // текущий прин€тый символ
volatile char RX_BUF[RX_BUF_SIZE] = {'\0'};

#define	MAX_MODBUS_BUF 50
volatile char modbus_buf[MAX_MODBUS_BUF];

RGB_t led_red;
RGB_t led_blue;
RGB_t led_green;
//------------------------------------------------------------
#define CMD_SET_DELAY_MS    1
#define CMD_SET_BRIGHTNESS  2

#define CMD_01 10
#define CMD_02 20
#define CMD_03 30
#define CMD_04 40
#define CMD_05 50

enum E_MODE
{
	MODE_01 = 0,
	MODE_02 = 1,
	MODE_03 = 2,
	MODE_04 = 3,
	MODE_05 = 4,

	MODE_MAX = MODE_05
};
//------------------------------------------------------------
#pragma pack (push, 1)

typedef struct HEADER
{
	uint8_t   addr;
	uint8_t   cmd;
	uint16_t  len;
	uint8_t   data[];
} header_t;

typedef struct MODE
{
	uint8_t  gBrightness;
	uint16_t delay_ms;
} mode_t;

union DATA
{
	struct MEMORY
	{
		uint8_t current_mode;
		struct MODE modes[MODE_MAX + 1];
	} memory_t;
	char buf[sizeof(struct MEMORY)];
} data_t;

#pragma pack(pop)
//------------------------------------------------------------
uint16_t crc16(uint8_t *pcBlock, uint8_t len)
{
	uint16_t a, crc = 0xFFFF;
	int i;
	while (len--)
	{
		a = *pcBlock++ << 8;
		crc ^= a;
		for (i = 0; i < 8; i++)
			crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
	}
	return crc;
}
//------------------------------------------------------------
void clear_RXBuffer(void)
{
	for (RXi=0; RXi<RX_BUF_SIZE; RXi++)
	{
		RX_BUF[(int)RXi] = '\0';
	}
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
		RX_BUF[(int)RXi] = RXc;
		RXi++;

		if (RXc != '\n')
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
uint8_t convert(uint8_t x)
{
	uint8_t res = 0;
	switch (x)
	{
	case '0':  res = 0x0;  break;
	case '1':  res = 0x1;  break;
	case '2':  res = 0x2;  break;
	case '3':  res = 0x3;  break;
	case '4':  res = 0x4;  break;
	case '5':  res = 0x5;  break;
	case '6':  res = 0x6;  break;
	case '7':  res = 0x7;  break;
	case '8':  res = 0x8;  break;
	case '9':  res = 0x9;  break;
	case 'A':  res = 0xA;  break;
	case 'B':  res = 0xB;  break;
	case 'C':  res = 0xC;  break;
	case 'D':  res = 0xD;  break;
	case 'E':  res = 0xE;  break;
	case 'F':  res = 0xF;  break;
	default:
		break;
	}
	return res;
}
//------------------------------------------------------------
void convert_data_to_ascii(uint8_t data, char *hi_str, char *lo_str)
{
	uint8_t hi = (data >> 4) & 0x0F;
	uint8_t lo = (data & 0x0F);

	switch (hi)
	{
	case 0x00: *hi_str = '0'; break;
	case 0x01: *hi_str = '1'; break;
	case 0x02: *hi_str = '2'; break;
	case 0x03: *hi_str = '3'; break;
	case 0x04: *hi_str = '4'; break;
	case 0x05: *hi_str = '5'; break;
	case 0x06: *hi_str = '6'; break;
	case 0x07: *hi_str = '7'; break;
	case 0x08: *hi_str = '8'; break;
	case 0x09: *hi_str = '9'; break;
	case 0x0A: *hi_str = 'A'; break;
	case 0x0B: *hi_str = 'B'; break;
	case 0x0C: *hi_str = 'C'; break;
	case 0x0D: *hi_str = 'D'; break;
	case 0x0E: *hi_str = 'E'; break;
	case 0x0F: *hi_str = 'F'; break;
	default:
		break;
	}

	switch (lo)
	{
	case 0x00: *lo_str = '0'; break;
	case 0x01: *lo_str = '1'; break;
	case 0x02: *lo_str = '2'; break;
	case 0x03: *lo_str = '3'; break;
	case 0x04: *lo_str = '4'; break;
	case 0x05: *lo_str = '5'; break;
	case 0x06: *lo_str = '6'; break;
	case 0x07: *lo_str = '7'; break;
	case 0x08: *lo_str = '8'; break;
	case 0x09: *lo_str = '9'; break;
	case 0x0A: *lo_str = 'A'; break;
	case 0x0B: *lo_str = 'B'; break;
	case 0x0C: *lo_str = 'C'; break;
	case 0x0D: *lo_str = 'D'; break;
	case 0x0E: *lo_str = 'E'; break;
	case 0x0F: *lo_str = 'F'; break;
	default:
		break;
	}
}
//------------------------------------------------------------
uint8_t convert_ascii_to_value(uint8_t hi, uint8_t lo)
{
	uint8_t b_hi = convert(hi);
	uint8_t b_lo = convert(lo);

	uint8_t r_byte = (b_hi << 4) | b_lo;
	return r_byte;
}
//------------------------------------------------------------
bool check_packet(void)
{
	return true;
}
//------------------------------------------------------------
bool analize_packet(void)
{
	if (RXi == 0)
	{
		return false;
	}
	if (RXi % 2)
	{
		return false;
	}

	int index_modbus_buf = 0;
	int n = 0;
	for (n = 0; n < RXi; n += 2)
	{
		modbus_buf[index_modbus_buf] = convert_ascii_to_value(RX_BUF[n], RX_BUF[n + 1]);
		index_modbus_buf++;
		if (index_modbus_buf > MAX_MODBUS_BUF)
		{
			return false;
		}
	}
	if (index_modbus_buf < sizeof(struct HEADER))
	{
		return false;
	}
	//---
	bool ok = check_packet();
	if(ok)
	{
		struct HEADER *header = (struct HEADER *)&modbus_buf;
		uint8_t cmd = header->cmd;
		switch(cmd)
		{
		case CMD_SET_DELAY_MS:
			send_answer(CMD_SET_DELAY_MS);
			break;

		case CMD_SET_BRIGHTNESS:
			send_answer(CMD_SET_BRIGHTNESS);
			break;

		case CMD_01:
			data_t.memory_t.current_mode = MODE_01;
			send_answer(CMD_01);
			break;

		case CMD_02:
			data_t.memory_t.current_mode = MODE_02;
			send_answer(CMD_02);
			break;

		case CMD_03:
			data_t.memory_t.current_mode = MODE_03;
			send_answer(CMD_03);
			break;

		case CMD_04:
			data_t.memory_t.current_mode = MODE_04;
			send_answer(CMD_04);
			break;

		case CMD_05:
			data_t.memory_t.current_mode = MODE_05;
			send_answer(CMD_05);
			break;

		default:
			ok = false;
			break;
		}
	}

	return true;
}
//------------------------------------------------------------
void send_answer(uint8_t cmd)
{
	header_t header;
	header.addr = 0;
	header.cmd  = cmd;
	header.len  = 0;

	char send_buffer[100] = { 0 };
	int index = 0;
	int n=0;

	char *begin = (char *)&header;

	send_buffer[index++] = ':';
	for(n=0; n<sizeof(header); n++)
	{
		char hi = '0';
		char lo = '0';
		convert_data_to_ascii(*begin + n, &hi, &lo);
		send_buffer[index++] = hi;
		send_buffer[index++] = lo;
	}
	send_buffer[index] = '\n';
	USARTSend(send_buffer);
}
//------------------------------------------------------------
void delay_ms(uint32_t ms)
{
	volatile uint32_t nCount;
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq (&RCC_Clocks);

	nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
	for (; nCount!=0; nCount--);
}
//------------------------------------------------------------
void f_01(void)
{
	int n = 0;
	for (n = 0; n < MAX_ADDRESS_FRONT; n++)
	{
		set_left_pixel_front(n,  led_red);
		set_right_pixel_front(n, led_blue);
	}
	for (n = 0; n < MAX_ADDRESS_BACK; n++)
	{
		set_left_pixel_back(n,  led_red);
		set_right_pixel_back(n, led_blue);
	}
	for (n = 0; n < (SIZE_MATRIX_3 / 2); n++)
	{
		set_horizontal_pixel(n, led_red);
		set_horizontal_pixel(n + (SIZE_MATRIX_3 / 2), led_blue);
	}
	show_leds();
	pause(data_t.memory_t.modes[data_t.memory_t.current_mode].delay_ms);

	for (n = 0; n < MAX_ADDRESS_FRONT; n++)
	{
		set_left_pixel_front(n,  led_blue);
		set_right_pixel_front(n, led_red);
	}
	for (n = 0; n < MAX_ADDRESS_BACK; n++)
	{
		set_left_pixel_back(n,  led_blue);
		set_right_pixel_back(n, led_red);
	}
	for (n = 0; n < (SIZE_MATRIX_3 / 2); n++)
	{
		set_horizontal_pixel(n, led_blue);
		set_horizontal_pixel(n + (SIZE_MATRIX_3 / 2), led_red);
	}
	show_leds();
	pause(data_t.memory_t.modes[data_t.memory_t.current_mode].delay_ms);
}
//------------------------------------------------------------
void f_02(void)
{
	int n;
	for (n = 0; n < MAX_ADDRESS_FRONT; n++)
	{
		set_left_pixel_front(n,  led_red);
		set_right_pixel_front(n, led_blue);
	}
	for (n = 0; n < MAX_ADDRESS_BACK; n++)
	{
		set_left_pixel_back(n,  led_red);
		set_right_pixel_back(n, led_blue);
	}
	for (n = 0; n < (SIZE_MATRIX_3 / 2); n++)
	{
		set_horizontal_pixel(n, led_red);
		set_horizontal_pixel(n + (SIZE_MATRIX_3 / 2), led_blue);
	}
	show_leds();
	pause(data_t.memory_t.modes[data_t.memory_t.current_mode].delay_ms);

	for (n = 0; n < MAX_ADDRESS_FRONT; n++)
	{
		set_left_pixel_front(n,  led_blue);
		set_right_pixel_front(n, led_red);
	}
	for (n = 0; n < MAX_ADDRESS_BACK; n++)
	{
		set_left_pixel_back(n,  led_blue);
		set_right_pixel_back(n, led_red);
	}
	for (n = 0; n < (SIZE_MATRIX_3 / 2); n++)
	{
		set_horizontal_pixel(n, led_blue);
		set_horizontal_pixel(n + (SIZE_MATRIX_3 / 2), led_red);
	}
	show_leds();
	pause(data_t.memory_t.modes[data_t.memory_t.current_mode].delay_ms);
}
//------------------------------------------------------------
void f_03(void)
{
	unsigned int n;
	for (n = 0; n < MAX_SIZE; n += 2)
	{
		set_left_pixel(n,       led_red);
		set_left_pixel(n + 1,   led_blue);
		set_right_pixel(n,      led_red);
		set_right_pixel(n + 1,  led_blue);
		set_horizontal(led_red);
	}
	show_leds();
	pause(data_t.memory_t.modes[data_t.memory_t.current_mode].delay_ms);

	for (n = 0; n < MAX_SIZE; n += 2)
	{
		set_left_pixel(n,       led_blue);
		set_left_pixel(n + 1,   led_red);
		set_right_pixel(n,      led_blue);
		set_right_pixel(n + 1,  led_red);
		set_horizontal(led_blue);
	}
	show_leds();
	pause(data_t.memory_t.modes[data_t.memory_t.current_mode].delay_ms);
}
//------------------------------------------------------------
void f_04(void)
{
	int n;
	if (current_led_front < MAX_ADDRESS_FRONT)
	{
		current_led_front++;
	}
	else
	{
		current_led_front = 0;
	}

	if (current_led_back < MAX_ADDRESS_BACK)
	{
		current_led_back++;
	}
	else
	{
		current_led_back = 0;
	}

	clear_leds();

	for (n = 0; n < 5; n++)
	{
		set_left_pixel_front(current_led_front + n,  led_red);
		set_right_pixel_front(current_led_front + n, led_red);

		set_left_pixel_back(current_led_back + n,  led_blue);
		set_right_pixel_back(current_led_back + n, led_blue);
	}

	show_leds();
	pause(data_t.memory_t.modes[data_t.memory_t.current_mode].delay_ms);
}
//------------------------------------------------------------
void f_05(void)
{
	int n;
	if (current_led < MAX_SIZE)
	{
		current_led++;
	}
	else
	{
		current_led = 0;
	}

	clear_leds();
	for (n = 0; n < 5; n++)
	{
		set_left_pixel(current_led + n,   led_red);
		set_right_pixel(current_led + n,  led_blue);
	}

	show_leds();
	pause(data_t.memory_t.modes[data_t.memory_t.current_mode].delay_ms);
}
//------------------------------------------------------------
void test_hsv(void)
{
	HSV_t hsv_buffer[DEBUG_SIZE];

	//TODO test
	int n = 0;
	int tone = 0;

	while(true)
	{
		if(tone < 360)
			tone++;
		else
			tone = 0;

		HSV_t led;
		led.h = tone;   // тон          0..360
		led.s = 100;    // насыщенность 0..100
		led.v = 100;    // €ркость      0..100 (min 12)

		for(n=0; n<DEBUG_SIZE; n++)
		{
			hsv_buffer[n] = led;
		}
		ws2812b_SendHSV(hsv_buffer, DEBUG_SIZE);
		delay_ms(50);
	}
}
//------------------------------------------------------------
void check_adv(void)
{
	if (strncmp(strupr(RX_BUF), "+\r", 2) == 0)
	{
		if(led_index < MAX_SIZE)
		{
			led_index ++;
		}
		while (!ws2812b_IsReady()); // wait
		ws2812b_SendRGB(&led_buffer[led_index], DEBUG_SIZE);
	}
	if (strncmp(strupr(RX_BUF), "-\r", 2) == 0)
	{
		if(led_index > 0)
		{
			led_index --;
		}
		while (!ws2812b_IsReady()); // wait
		ws2812b_SendRGB(&led_buffer[led_index], DEBUG_SIZE);
	}
}
//------------------------------------------------------------
int main(void)
{
	ws2812b_Init();

	led_red.r = color;
	led_red.g = 0;
	led_red.b = 0;

	led_blue.r = 0;
	led_blue.g = 0;
	led_blue.b = color;

	led_green.r = 0;
	led_green.g = color;
	led_green.b = 0;

	// Initialize USART
	usart_init();
	USARTSend(" USART1 is ready.\n");

	init_leds();

	//clear_leds();
	while (!ws2812b_IsReady()); // wait
	//test_hsv();	//TODO

	ws2812b_SendRGB(led_buffer, DEBUG_SIZE);

	led_index = 0;
	while (1)
	{
		if (RX_FLAG_END_LINE == 1)
		{
			// Reset END_LINE Flag
			RX_FLAG_END_LINE = 0;

			analize_packet();
			check_adv();	//TODO
			clear_RXBuffer();
		}
	}
}
//------------------------------------------------------------
