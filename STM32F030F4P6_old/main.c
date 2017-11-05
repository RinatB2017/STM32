//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
//#include "misc.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_usart.h"

#include "pelco.h"
#include "packets.h"
//--------------------------------------------------------------------------------
#define bool	char
#define true	1
#define false	0
//--------------------------------------------------------------------------------
void f_test();
void f_reset();
void f_read();
void f_write();

void send_byte(uint8_t data);
void send_data();

void work();
void command();

uint8_t convert_ascii_to_value(uint8_t hi, uint8_t lo);
void convert_data_to_ascii(uint8_t data, uint8_t *hi_str, uint8_t *lo_str);
//--------------------------------------------------------------------------------
#define MAX_BUF	200
uint8_t		dirty_buf[MAX_BUF]; // текстовый буфер AABBCC
uint8_t		buf[MAX_BUF];
uint16_t	index_buf = 0;
//--------------------------------------------------------------------------------
uint32_t    addr_cam_32 = 0;                // адрес камеры
uint16_t    time_interval_16 = 0;           // интервал дворника
uint32_t    time_washout_32 = 0;            // время помывки
uint32_t    time_pause_washout_32 = 0;      // время между помывками
uint32_t    preset_washout_32 = 0;          // пресет помывки
uint32_t    time_preset_washout_32 = 0;     // времен помывки
//--------------------------------------------------------------------------------
long sys_tick_cnt = 0;
char flag = 0;
//--------------------------------------------------------------------------------
void blink_ON()
{
	 GPIO_SetBits(GPIOA,	GPIO_Pin_0);
}
//--------------------------------------------------------------------------------
void blink_OFF()
{
	 GPIO_ResetBits(GPIOA,	GPIO_Pin_0);
}
//--------------------------------------------------------------------------------
void pump_ON()
{
	 GPIO_SetBits(GPIOC,	GPIO_Pin_8);
}
//--------------------------------------------------------------------------------
void pump_OFF()
{
	 GPIO_ResetBits(GPIOC,	GPIO_Pin_8);
}
//--------------------------------------------------------------------------------
void relay_ON()
{
	 GPIO_SetBits(GPIOC,	GPIO_Pin_9);
}
//--------------------------------------------------------------------------------
void relay_OFF()
{
	 GPIO_ResetBits(GPIOC,	GPIO_Pin_9);
}
//--------------------------------------------------------------------------------
bool check_RAIN()
{
	return (GPIO_ReadInputData(GPIOA) && GPIO_Pin_3);
}
//--------------------------------------------------------------------------------
bool check_LEVEL()
{
	return (GPIO_ReadInputData(GPIOA) && GPIO_Pin_3);
}
//--------------------------------------------------------------------------------
uint8_t get_address()
{
	union U_BYTE temp;
	temp.value = 0;
	temp.bites.bit0 = (GPIO_ReadInputData(GPIOA) && GPIO_Pin_0);
	temp.bites.bit1 = (GPIO_ReadInputData(GPIOA) && GPIO_Pin_1);
	temp.bites.bit2 = (GPIO_ReadInputData(GPIOA) && GPIO_Pin_2);
	temp.bites.bit3 = (GPIO_ReadInputData(GPIOA) && GPIO_Pin_3);
	temp.bites.bit4 = (GPIO_ReadInputData(GPIOA) && GPIO_Pin_4);

	return temp.value;
}
//--------------------------------------------------------------------------------
uint16_t crc16(uint8_t *pcBlock, uint8_t len)
{
	uint16_t a, crc = 0xFFFF;
	while(len--)
	{
		a = *pcBlock++ << 8;
		crc ^= a;
		for(int i=0; i<8; i++)
			crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
	}
	return crc;
}
//--------------------------------------------------------------------------------
void read_RS485()
{
	GPIO_ResetBits(GPIOA,	GPIO_Pin_11);
}
//--------------------------------------------------------------------------------
void write_RS485()
{
	GPIO_SetBits(GPIOA,	GPIO_Pin_11);
}
//--------------------------------------------------------------------------------
void init_gpio()
{
    // Enable GPIO clock
	//RCC_APB2PeriphClockCmd(RCC_APB1Periph_AFIO,		ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, 	ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,		ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, 		ENABLE);

    // Configure pin as output push-pull (LED)
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |
    		GPIO_Pin_1 |
    		GPIO_Pin_2 |
    		GPIO_Pin_3 |
			GPIO_Pin_4 |
			GPIO_Pin_5 |
			GPIO_Pin_6 |
			GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //Инициализация вывода PA9 - USART1_Tx
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 			// Настройки вывода PA9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 	// Скорость порта максимальная
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	 	// Режим альтернативной функции, выход Push-Pull
    GPIO_Init(GPIOA, &GPIO_InitStructure); 				// Заданные настройки сохраняем в регистрах GPIOА

    //Инициализация вывода PA10 - USART1_Rx
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 			// Настройки вывода PA10
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;	 	// Input floating
    GPIO_Init(GPIOA, &GPIO_InitStructure); 				// Заданные настройки сохраняем в регистрах GPIOА

    read_RS485();
}
//--------------------------------------------------------------------------------
void init_usart()
{
	USART_InitTypeDef	USART_InitStruct;
	NVIC_InitTypeDef	NVIC_InitStructure;

	//Инициализация USART1
	USART_InitStruct.USART_BaudRate = 9600; 						// Скорость обмена 9600 бод
	USART_InitStruct.USART_WordLength = USART_WordLength_8b; 		// Длина слова 8 бит
	USART_InitStruct.USART_StopBits = USART_StopBits_1; 			// 1 стоп-бит
	USART_InitStruct.USART_Parity = USART_Parity_No ; 				// Без проверки четности
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	// Без аппаратного контроля
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 	// Включен передатчик и приемник USART1
	USART_Init(USART1, &USART_InitStruct); 							// Заданные настройки сохраняем в регистрах USART1

	USART_Cmd(USART1, ENABLE); //Включаем USART1

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	// прерывание на пришедшие данные в буфер;
	//USART_ITConfig(USART1, USART_IT_TC, ENABLE);	// прерывание на окончание передачи

	NVIC_EnableIRQ (USART1_IRQn);           		// разрешить прерывания от USART1
}
//--------------------------------------------------------------------------------
void USART1_IRQHandler()
{
    /* Прерывание по приему байта по USART */
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        USART_ClearFlag(USART1, USART_IT_RXNE);

        // функция обработчик
        uint8_t s = USART_ReceiveData(USART1);
		switch(s)
		{
		case ':':
			index_buf = 0;
			break;

		case '\n':
			command();
			index_buf = 0;
			break;

		default:
			if(index_buf < MAX_BUF)
			{
				dirty_buf[index_buf] = s;
				index_buf++;
			}
			break;
		}
    }

    if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
    {
    	index_buf = 0;
    }
    USART_ClearITPendingBit(USART1, USART_IT_TC);
}
//--------------------------------------------------------------------------------
void work()
{
	check_LEVEL();
	check_RAIN();

	flag = !flag;
	if(flag)
	{
		blink_ON();
		pump_ON();
		relay_OFF();
	}
	else
	{
		blink_OFF();
		pump_OFF();
		relay_ON();
	}
}
//--------------------------------------------------------------------------------
void command()
{
	  if(index_buf == 0)
	  {
	    return;
	  }
	  if(index_buf % 2)
	  {
	    return;
	  }

	  int temp_index_buf = 0;
	  for(int n=0; n<index_buf; n+=2)
	  {
		  buf[temp_index_buf] = convert_ascii_to_value(dirty_buf[n], dirty_buf[n+1]);
		  temp_index_buf++;
	  }
	  index_buf = temp_index_buf;

	  struct HEADER *packet = (struct HEADER *)&buf;
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
void SysTick_Handler(void)
{
	sys_tick_cnt++;
	if(sys_tick_cnt > 500)
	{
		sys_tick_cnt = 0;
		//---
		work();
    	//---
	}
}
//--------------------------------------------------------------------------------
void send_byte(uint8_t data)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET){}
	USART_SendData(USART1, data);
}
//--------------------------------------------------------------------------------
void send_data()
{
	uint8_t hi_str = 0;
	uint8_t lo_str = 0;

    write_RS485();

    send_byte(':');
    for(int n=0; n<index_buf; n++)
    {
    	convert_data_to_ascii(buf[n], &hi_str, &lo_str);
    	send_byte(hi_str);
    	send_byte(lo_str);
    }
    send_byte('\n');

	read_RS485();
}
//--------------------------------------------------------------------------------
void f_read()
{
	union QUESTION_READ *packet = (union QUESTION_READ *)&buf;

	uint16_t crc = crc16((uint8_t *)&buf, sizeof(union QUESTION_READ) - 2);
	if(crc != packet->body.crc16)
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

	for(int n=0; n<sizeof(answer); n++)
	{
		buf[n] = answer.buf[n];
	}
	index_buf = sizeof(answer);
	send_data();
}
//--------------------------------------------------------------------------------
void f_write()
{
	union QUESTION_WRITE *packet = (union QUESTION_WRITE *)&buf;

	uint16_t crc = crc16((uint8_t *)&buf, sizeof(union QUESTION_WRITE) - 2);
	if(crc != packet->body.crc16)
	{
		return;
	}

	addr_cam_32 = packet->body.addr_cam_32;                			// адрес камеры
	time_interval_16 = packet->body.time_interval_16;           	// интервал дворника
	time_washout_32 = packet->body.time_washout_32;            		// время помывки
	time_pause_washout_32 = packet->body.time_pause_washout_32;     // время между помывками
	preset_washout_32 = packet->body.preset_washout_32;          	// пресет помывки
	time_preset_washout_32 = packet->body.time_preset_washout_32;	// времен помывки

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

	for(int n=0; n<sizeof(answer); n++)
	{
		buf[n] = answer.buf[n];
	}
	index_buf = sizeof(answer);
	send_data();
}
//--------------------------------------------------------------------------------
void f_test()
{
	union QUESTION_TEST *packet = (union QUESTION_TEST *)&buf;

	uint16_t crc = crc16((uint8_t *)&buf, sizeof(union QUESTION_TEST) - 2);
	if(crc != packet->body.crc16)
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

	for(int n=0; n<sizeof(answer); n++)
	{
		buf[n] = answer.buf[n];
	}
	index_buf = sizeof(answer);
	send_data();
}
//--------------------------------------------------------------------------------
void f_reset()
{
	union QUESTION_RESET *packet = (union QUESTION_RESET *)&buf;

	uint16_t crc = crc16((uint8_t *)&buf, sizeof(union QUESTION_RESET) - 2);
	if(crc != packet->body.crc16)
	{
		return;
	}

	union ANSWER_RESET answer;

	answer.body.header.prefix_16 = packet->body.header.prefix_16;
	answer.body.header.addr_8 = packet->body.header.addr_8;
	answer.body.header.cmd_8 = packet->body.header.cmd_8;
	answer.body.header.len_16 = packet->body.header.len_16;
	answer.body.data = packet->body.data;
	answer.body.crc16 = crc16((uint8_t *)&answer.buf, sizeof(union ANSWER_RESET) - 2);

	for(int n=0; n<sizeof(answer); n++)
	{
		buf[n] = answer.buf[n];
	}
	index_buf = sizeof(answer);
	send_data();
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
int main(void)
{
	__enable_irq ();
	SysTick_Config(SystemCoreClock / 1000);

	init_gpio();
	init_usart();

    while(1)
    {
    }
}
//--------------------------------------------------------------------------------
