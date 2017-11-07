//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>

#include <stdio.h>

#include "packets.h"

#include "usart.h"
#include "flash.h"
//--------------------------------------------------------------------------------
#define	true	1
#define false	!true
#define	bool	char
//--------------------------------------------------------------------------------
uint8_t convert_ascii_to_value(uint8_t hi, uint8_t lo);
uint16_t crc16(uint8_t *pcBlock, uint8_t len);
uint8_t get_address(void);
void send_data(void);
void send_byte(uint8_t b);
void blink_ON(void);
void blink_OFF(void);
void pump_ON(void);
void pump_OFF(void);
void relay_ON(void);
void relay_OFF(void);
void wash(bool value);
bool check_LEVEL(void);
bool check_RAIN(void);
void Delay_ms(uint32_t ms);
void command(void);
void work(void);
void f_read(void);
void f_write(void);
void f_test(void);
void f_reset(void);
void run_wiper(void);
void camera_Save_position (char preset);
void camera_Move_position (char preset);
void camera_Wiper (void);
void camera_Run_Tur_1 (void);
void write_RS485(void);
void read_RS485(void);
void write_FLASH(void);
void read_FLASH(void);
//--------------------------------------------------------------------------------
long sys_tick_cnt = 0;
char flag = 0;
volatile char temp_index_buf[RX_BUF_SIZE] = { 0 };
//--------------------------------------------------------------------------------
char Pelco[20] = { 0xFF, 0, 0, 0, 0, 0 };
//--------------------------------------------------------------------------------
uint32_t    addr_cam_32 = 0;                // адрес камеры
uint16_t    time_interval_16 = 0;           // интервал дворника
uint32_t    time_washout_32 = 0;            // время помывки
uint32_t    time_pause_washout_32 = 0;      // время между помывками
uint32_t    preset_washout_32 = 0;          // пресет помывки
uint32_t    time_preset_washout_32 = 0;     // времен помывки
//--------------------------------------------------------------------------------
long cnt_second = 0;
//--------------------------------------------------------------------------------
/*
 * если пошел дождь, то поведение такое же, как и при помывке, только не дергаем камерой
 */
enum {
	STATUS_IDLE = 0,
	STATUS_RAIN,			// дождь
	STATUS_WASHOUT,			// моемся
	STATUS_WASHOUT_PAUSE	// пауза
};
int state = STATUS_IDLE;
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

			blink_ON();
			work();
		}
		else
		{
			GPIO_ResetBits(GPIOC,	GPIO_Pin_8);
			GPIO_SetBits(GPIOC,		GPIO_Pin_9);

			blink_OFF();
		}
	}
}
//--------------------------------------------------------------------------------
void USART1_IRQHandler(void)
{
	if ((USART1->SR & USART_FLAG_RXNE) != (u16)RESET)
	{
		char s = USART_ReceiveData(USART1);
		switch(s)
		{
		case ':':
			RX_buffer_len = 0;
			break;

		case '\n':
			command();
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
void work()
{
	bool flag_lvl  = check_LEVEL();
	bool flag_rain = check_RAIN();

	// мало воды
	if(flag_lvl)
	{
		pump_ON();
	}
	else
	{
		pump_OFF();
	}

	if(flag_rain)
	{
		state = STATUS_RAIN;
	}

	switch(state)
	{
	case STATUS_IDLE:
		cnt_second = 0;
		state = STATUS_WASHOUT;
		break;

	case STATUS_RAIN:			// дождь
		if(flag_rain == false)
		{
			state = STATUS_IDLE;
			break;
		}
		if(cnt_second < time_interval_16)
		{
			cnt_second++;
		}
		else
		{
			run_wiper();
			cnt_second = 0;
		}
		break;

	case STATUS_WASHOUT:		// моемся
		wash(true);
		if(!(cnt_second % time_interval_16))
		{
			run_wiper();
		}
		if(cnt_second >= time_washout_32)
		{
			cnt_second = 0;
			state = STATUS_WASHOUT_PAUSE;
		}
		cnt_second++;
		break;

	case STATUS_WASHOUT_PAUSE:	// пауза
		wash(false);
		if(cnt_second >= time_pause_washout_32)
		{
			cnt_second = 0;
			state = STATUS_WASHOUT;
		}
		cnt_second++;
		break;

	default:
		state = STATUS_IDLE;
		break;
	}
}
//--------------------------------------------------------------------------------
void command(void)
{
	int n = 0;
	if(RX_buffer_len == 0)
	{
		return;
	}
	if(RX_buffer_len % 2)
	{
		return;
	}

	int temp_index_buf = 0;
	for(n=0; n<RX_buffer_len; n+=2)
	{
		RX_buffer[temp_index_buf] = convert_ascii_to_value(RX_buffer[n], RX_buffer[n+1]);
		temp_index_buf++;
	}
	RX_buffer_len = temp_index_buf;

	struct HEADER *packet = (struct HEADER *)&RX_buffer;
	uint8_t cmd = packet->cmd_8;
	switch(cmd)
	{
	case CMD_READ:
		f_read();
		break;
	case CMD_WRITE:
		f_write();
		break;
	case CMD_RESET:
		f_reset();
		break;
	case CMD_TEST:
		f_test();
		break;
	default:
		//logging(String("unknown cmd ") + String(cmd, HEX));
		break;
	}
}
//--------------------------------------------------------------------------------
void f_read()
{
	int n = 0;
	union QUESTION_READ *packet = (union QUESTION_READ *)&RX_buffer;

	uint16_t crc = crc16((uint8_t *)&RX_buffer, sizeof(union QUESTION_READ) - 2);
	if(crc != packet->body.crc16)
	{
		return;
	}
	uint8_t addr = get_address();
	if(addr != packet->body.header.addr_8)
	{
		return;
	}

	union ANSWER_READ answer;

	answer.body.header.prefix_16 = packet->body.header.prefix_16;
	answer.body.header.addr_8 = packet->body.header.addr_8;
	answer.body.header.cmd_8 = packet->body.header.cmd_8;
	answer.body.header.len_16 = packet->body.header.len_16;

	answer.body.addr_cam_32 = addr_cam_32;                			// адрес камеры
	answer.body.time_interval_16 = time_interval_16;           		// интервал дворника
	answer.body.time_washout_32 = time_washout_32;            		// время помывки
	answer.body.time_pause_washout_32 = time_pause_washout_32;     	// время между помывками
	answer.body.preset_washout_32 = preset_washout_32;          	// пресет помывки
	answer.body.time_preset_washout_32 = time_preset_washout_32;	// времен помывки

	answer.body.crc16 = crc16((uint8_t *)&answer.buf, sizeof(union ANSWER_READ) - 2);

	for(n=0; n<sizeof(answer); n++)
	{
		TX_buffer[n] = answer.buf[n];
	}
	TX_buffer_len = sizeof(answer);
	send_data();
}
//--------------------------------------------------------------------------------
void f_write()
{
	int n = 0;
	union QUESTION_WRITE *packet = (union QUESTION_WRITE *)&RX_buffer;

	uint16_t crc = crc16((uint8_t *)&RX_buffer, sizeof(union QUESTION_WRITE) - 2);
	if(crc != packet->body.crc16)
	{
		return;
	}
	uint8_t addr = get_address();
	if(addr != packet->body.header.addr_8)
	{
		return;
	}

	addr_cam_32 = packet->body.addr_cam_32;                			// адрес камеры
	time_interval_16 = packet->body.time_interval_16;           	// интервал дворника
	time_washout_32 = packet->body.time_washout_32;            		// время помывки
	time_pause_washout_32 = packet->body.time_pause_washout_32;     // время между помывками
	preset_washout_32 = packet->body.preset_washout_32;          	// пресет помывки
	time_preset_washout_32 = packet->body.time_preset_washout_32;	// времен помывки

	//---
	write_FLASH();
	//---

	union ANSWER_WRITE answer;

	answer.body.header.prefix_16 = packet->body.header.prefix_16;
	answer.body.header.addr_8 = packet->body.header.addr_8;
	answer.body.header.cmd_8 = packet->body.header.cmd_8;
	answer.body.header.len_16 = packet->body.header.len_16;

	answer.body.addr_cam_32 = addr_cam_32;                			// адрес камеры
	answer.body.time_interval_16 = time_interval_16;           		// интервал дворника
	answer.body.time_washout_32 = time_washout_32;            		// время помывки
	answer.body.time_pause_washout_32 = time_pause_washout_32;     	// время между помывками
	answer.body.preset_washout_32 = preset_washout_32;          	// пресет помывки
	answer.body.time_preset_washout_32 = time_preset_washout_32;	// времен помывки

	answer.body.crc16 = crc16((uint8_t *)&answer.buf, sizeof(union ANSWER_WRITE) - 2);

	for(n=0; n<sizeof(answer); n++)
	{
		TX_buffer[n] = answer.buf[n];
	}
	TX_buffer_len = sizeof(answer);
	send_data();
}
//--------------------------------------------------------------------------------
void f_test()
{
	int n = 0;
	union QUESTION_TEST *packet = (union QUESTION_TEST *)&RX_buffer;

	uint16_t crc = crc16((uint8_t *)&RX_buffer, sizeof(union QUESTION_TEST) - 2);
	if(crc != packet->body.crc16)
	{
		return;
	}
	uint8_t addr = get_address();
	if(addr != packet->body.header.addr_8)
	{
		return;
	}

	union ANSWER_TEST answer;

	answer.body.header.prefix_16 = packet->body.header.prefix_16;
	answer.body.header.addr_8 = packet->body.header.addr_8;
	answer.body.header.cmd_8 = packet->body.header.cmd_8;
	answer.body.header.len_16 = packet->body.header.len_16;
	answer.body.data = packet->body.data;
	answer.body.crc16 = crc16((uint8_t *)&answer.buf, sizeof(union ANSWER_TEST) - 2);

	for(n=0; n<sizeof(answer); n++)
	{
		TX_buffer[n] = answer.buf[n];
	}
	TX_buffer_len = sizeof(answer);
	send_data();
}
//--------------------------------------------------------------------------------
void f_reset()
{
	int n = 0;
	union QUESTION_RESET *packet = (union QUESTION_RESET *)&RX_buffer;

	uint16_t crc = crc16((uint8_t *)&RX_buffer, sizeof(union QUESTION_RESET) - 2);
	if(crc != packet->body.crc16)
	{
		return;
	}
	uint8_t addr = get_address();
	if(addr != packet->body.header.addr_8)
	{
		return;
	}

	//---
	time_interval_16 = 5;           // интервал дворника
	time_washout_32 = 10;           // время помывки
	time_pause_washout_32 = 20;		// время между помывками
	preset_washout_32 = 0;          // пресет помывки
	time_preset_washout_32 = 0;     // времен помывки
	write_FLASH();
	state = STATUS_IDLE;
	//---

	union ANSWER_RESET answer;

	answer.body.header.prefix_16 = packet->body.header.prefix_16;
	answer.body.header.addr_8 = packet->body.header.addr_8;
	answer.body.header.cmd_8 = packet->body.header.cmd_8;
	answer.body.header.len_16 = packet->body.header.len_16;
	answer.body.data = 0;
	answer.body.crc16 = crc16((uint8_t *)&answer.buf, sizeof(union ANSWER_RESET) - 2);

	for(n=0; n<sizeof(answer); n++)
	{
		TX_buffer[n] = answer.buf[n];
	}
	TX_buffer_len = sizeof(answer);
	send_data();
}
//--------------------------------------------------------------------------------
void run_wiper(void)
{
	// запустим цикл дворника
	relay_ON();
	Delay_ms(200);
	relay_OFF();
}
//--------------------------------------------------------------------------------
void camera_Save_position (char preset)
{
	Pelco [6] = Pelco [1] ^ Pelco [2] ^Pelco [3] ^Pelco [4] ^Pelco [5] ;	// вычисление контрольной суммы

	write_RS485();
	send_byte(0xFF);
	send_byte(Pelco[1]);
	send_byte(Pelco[2]);
	send_byte(Pelco[3]);
	send_byte(Pelco[4]);
	send_byte(Pelco[5]);
	send_byte(Pelco[6]);
	read_RS485();
}
//--------------------------------------------------------------------------------
void camera_Move_position (char preset)
{
	Pelco [6] = Pelco [1] ^ Pelco [2] ^Pelco [3] ^Pelco [4] ^Pelco [5] ;	// вычисление контрольной суммы

	write_RS485();
	send_byte(0xFF);
	send_byte(Pelco[1]);
	send_byte(Pelco[2]);
	send_byte(Pelco[3]);
	send_byte(Pelco[4]);
	send_byte(Pelco[5]);
	send_byte(Pelco[6]);
	read_RS485();
}
//--------------------------------------------------------------------------------
void camera_Wiper (void)
{
	Pelco [6] = Pelco [1] ^ Pelco [2] ^Pelco [3] ^Pelco [4] ^Pelco [5] ;	// вычисление контрольной суммы

	write_RS485();
	send_byte(0xFF);
	send_byte(Pelco[1]);
	send_byte(Pelco[2]);
	send_byte(Pelco[3]);
	send_byte(Pelco[4]);
	send_byte(Pelco[5]);
	send_byte(Pelco[6]);
	read_RS485();
}
//--------------------------------------------------------------------------------
void camera_Run_Tur_1 (void)
{
	Pelco [6] = Pelco [1] ^ Pelco [2] ^Pelco [3] ^Pelco [4] ^Pelco [5] ;	// вычисление контрольной суммы

	write_RS485();
	send_byte(0xFF);
	send_byte(Pelco[1]);
	send_byte(Pelco[2]);
	send_byte(Pelco[3]);
	send_byte(Pelco[4]);
	send_byte(Pelco[5]);
	send_byte(Pelco[6]);
	read_RS485();
}
//--------------------------------------------------------------------------------
uint8_t convert_ascii_to_value(uint8_t hi, uint8_t lo)
{
	uint8_t b_hi = 0;
	uint8_t b_lo = 0;

	//---
	switch(hi)
	{
	case '0':  b_hi = 0x0;  break;
	case '1':  b_hi = 0x1;  break;
	case '2':  b_hi = 0x2;  break;
	case '3':  b_hi = 0x3;  break;
	case '4':  b_hi = 0x4;  break;
	case '5':  b_hi = 0x5;  break;
	case '6':  b_hi = 0x6;  break;
	case '7':  b_hi = 0x7;  break;
	case '8':  b_hi = 0x8;  break;
	case '9':  b_hi = 0x9;  break;
	case 'A':  b_hi = 0xA;  break;
	case 'B':  b_hi = 0xB;  break;
	case 'C':  b_hi = 0xC;  break;
	case 'D':  b_hi = 0xD;  break;
	case 'E':  b_hi = 0xE;  break;
	case 'F':  b_hi = 0xF;  break;
	default:
		break;
	}
	//---
	switch(lo)
	{
	case '0':  b_lo = 0x0;  break;
	case '1':  b_lo = 0x1;  break;
	case '2':  b_lo = 0x2;  break;
	case '3':  b_lo = 0x3;  break;
	case '4':  b_lo = 0x4;  break;
	case '5':  b_lo = 0x5;  break;
	case '6':  b_lo = 0x6;  break;
	case '7':  b_lo = 0x7;  break;
	case '8':  b_lo = 0x8;  break;
	case '9':  b_lo = 0x9;  break;
	case 'A':  b_lo = 0xA;  break;
	case 'B':  b_lo = 0xB;  break;
	case 'C':  b_lo = 0xC;  break;
	case 'D':  b_lo = 0xD;  break;
	case 'E':  b_lo = 0xE;  break;
	case 'F':  b_lo = 0xF;  break;
	default:
		break;
	}
	//---
	uint8_t r_byte = (b_hi << 4) | b_lo;
	return r_byte;
}
//--------------------------------------------------------------------------------
void convert_data_to_ascii(uint8_t data, uint8_t *hi_str, uint8_t *lo_str)
{
	uint8_t hi = (data >> 4) & 0x0F;
	uint8_t lo = (data & 0x0F);

	switch(hi)
	{
	case 0x00: *hi_str='0'; break;
	case 0x01: *hi_str='1'; break;
	case 0x02: *hi_str='2'; break;
	case 0x03: *hi_str='3'; break;
	case 0x04: *hi_str='4'; break;
	case 0x05: *hi_str='5'; break;
	case 0x06: *hi_str='6'; break;
	case 0x07: *hi_str='7'; break;
	case 0x08: *hi_str='8'; break;
	case 0x09: *hi_str='9'; break;
	case 0x0A: *hi_str='A'; break;
	case 0x0B: *hi_str='B'; break;
	case 0x0C: *hi_str='C'; break;
	case 0x0D: *hi_str='D'; break;
	case 0x0E: *hi_str='E'; break;
	case 0x0F: *hi_str='F'; break;
	default: break;
	}

	switch(lo)
	{
	case 0x00: *lo_str='0'; break;
	case 0x01: *lo_str='1'; break;
	case 0x02: *lo_str='2'; break;
	case 0x03: *lo_str='3'; break;
	case 0x04: *lo_str='4'; break;
	case 0x05: *lo_str='5'; break;
	case 0x06: *lo_str='6'; break;
	case 0x07: *lo_str='7'; break;
	case 0x08: *lo_str='8'; break;
	case 0x09: *lo_str='9'; break;
	case 0x0A: *lo_str='A'; break;
	case 0x0B: *lo_str='B'; break;
	case 0x0C: *lo_str='C'; break;
	case 0x0D: *lo_str='D'; break;
	case 0x0E: *lo_str='E'; break;
	case 0x0F: *lo_str='F'; break;
	default: break;
	}
}
//--------------------------------------------------------------------------------
uint16_t crc16(uint8_t *pcBlock, uint8_t len)
{
	int i = 0;
	uint16_t a, crc = 0xFFFF;
	while(len--)
	{
		a = *pcBlock++ << 8;
		crc ^= a;
		for(i=0; i<8; i++)
			crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
	}
	return crc;
}
//--------------------------------------------------------------------------------
uint8_t get_address(void)
{
	return 0;
}
//--------------------------------------------------------------------------------
void write_RS485(void)
{

}
//--------------------------------------------------------------------------------
void read_RS485(void)
{

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
	uint8_t hi_str = 0;
	uint8_t lo_str = 0;

	write_RS485();

	send_byte((char)':');
	for(n=0; n<TX_buffer_len; n++)
	{
		convert_data_to_ascii(TX_buffer[n], &hi_str, &lo_str);
		send_byte(hi_str);
		send_byte(lo_str);
	}
	send_byte((char)0x0D);

	read_RS485();
}
//--------------------------------------------------------------------------------
void blink_ON(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_0);
}
//--------------------------------------------------------------------------------
void blink_OFF(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_0);
}
//--------------------------------------------------------------------------------
void pump_ON(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_1);
}
//--------------------------------------------------------------------------------
void pump_OFF(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}
//--------------------------------------------------------------------------------
void relay_ON(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_2);
}
//--------------------------------------------------------------------------------
void relay_OFF(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_2);
}
//--------------------------------------------------------------------------------
void wash(bool value)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_6, value);
	GPIO_WriteBit(GPIOA, GPIO_Pin_7, !value);
}
//--------------------------------------------------------------------------------
bool check_LEVEL(void)
{
	return false;
}
//--------------------------------------------------------------------------------
bool check_RAIN(void)
{
	return false;
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
int main(void)
{
	__enable_irq ();
	SysTick_Config(SystemCoreClock / 1000);
	init_GPIO();
	usart_init();

	read_FLASH();
	while(1)
	{
	}
}
//--------------------------------------------------------------------------------
