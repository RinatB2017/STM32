//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
//#include "misc.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_usart.h"
#include "stm32f0xx_flash.h"

#include <string.h>

#include "pelco.h"
#include "packets.h"
//--------------------------------------------------------------------------------
#define bool	char
#define true	1
#define false	0
//--------------------------------------------------------------------------------
/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
/* Private define ------------------------------------------------------------*/
#define FLASH_PAGE_SIZE         ((uint32_t)0x00000400)   /* FLASH Page Size */
#define FLASH_USER_START_ADDR   ((uint32_t)0x08006000)   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ((uint32_t)0x08007000)   /* End @ of user Flash area */
#define DATA_32                 ((uint32_t)0x12345678)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t EraseCounter = 0x00, Address = 0x00;
uint32_t Data = 0x3210ABCD;
uint32_t NbrOfPage = 0x00;
__IO FLASH_Status FLASHStatus = FLASH_COMPLETE;
__IO TestStatus MemoryProgramStatus = PASSED;
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
#define MAX_BUF	100
uint8_t		dirty_buf[MAX_BUF]; // ��������� ����� AABBCC
uint8_t		buf[MAX_BUF];
uint16_t	index_buf = 0;
//--------------------------------------------------------------------------------
uint32_t    addr_cam_32 = 0;                // ����� ������
uint16_t    time_interval_16 = 0;           // �������� ��������
uint32_t    time_washout_32 = 0;            // ����� �������
uint32_t    time_pause_washout_32 = 0;      // ����� ����� ���������
uint32_t    preset_washout_32 = 0;          // ������ �������
uint32_t    time_preset_washout_32 = 0;     // ������ �������
//--------------------------------------------------------------------------------
/*
 * ���� ����� �����, �� ��������� ����� ��, ��� � ��� �������, ������ �� ������� �������
 */
enum {
	STATUS_IDLE = 0,
	STATUS_RAIN,			// �����
	STATUS_WASHOUT,			// ������
	STATUS_WASHOUT_PAUSE	// �����
};
int state = STATUS_IDLE;
//--------------------------------------------------------------------------------
long sys_tick_cnt = 0;
long cnt_flash = 0;
char flag = 0;
//--------------------------------------------------------------------------------
void blink_ON()
{
	GPIO_SetBits(GPIOB,	GPIO_Pin_1);
}
//--------------------------------------------------------------------------------
void blink_OFF()
{
	GPIO_ResetBits(GPIOB,	GPIO_Pin_1);
}
//--------------------------------------------------------------------------------
void pump_ON()
{
	GPIO_SetBits(GPIOA,	GPIO_Pin_0);
}
//--------------------------------------------------------------------------------
void pump_OFF()
{
	GPIO_ResetBits(GPIOA,	GPIO_Pin_0);
}
//--------------------------------------------------------------------------------
void relay_ON()
{
	GPIO_SetBits(GPIOA,	GPIO_Pin_5);
}
//--------------------------------------------------------------------------------
void relay_OFF()
{
	GPIO_ResetBits(GPIOA,	GPIO_Pin_5);
}
//--------------------------------------------------------------------------------
bool check_RAIN()
{
	return (GPIO_ReadInputData(GPIOA) & GPIO_Pin_6);
}
//--------------------------------------------------------------------------------
bool check_LEVEL()
{
	return (GPIO_ReadInputData(GPIOA) & GPIO_Pin_1);
}
//--------------------------------------------------------------------------------
uint8_t get_address()
{
	union U_BYTE temp;
	temp.value = 0;
	temp.bites.bit0 = (GPIO_ReadInputData(GPIOA) & GPIO_Pin_0);
	temp.bites.bit1 = (GPIO_ReadInputData(GPIOA) & GPIO_Pin_1);
	temp.bites.bit2 = (GPIO_ReadInputData(GPIOA) & GPIO_Pin_2);
	temp.bites.bit3 = (GPIO_ReadInputData(GPIOA) & GPIO_Pin_3);
	temp.bites.bit4 = (GPIO_ReadInputData(GPIOA) & GPIO_Pin_4);

	return temp.value;
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
void Delay_ms(uint32_t ms)
{
	volatile uint32_t nCount;
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq (&RCC_Clocks);

	nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
	for (; nCount!=0; nCount--);
}
//--------------------------------------------------------------------------------
void read_RS485()
{
	Delay_ms(20);
	GPIO_ResetBits(GPIOA,	GPIO_Pin_4);
}
//--------------------------------------------------------------------------------
void write_RS485()
{
	GPIO_SetBits(GPIOA,	GPIO_Pin_4);
}
//--------------------------------------------------------------------------------
void USART1_IRQHandler()
{
	/* ���������� �� ������ ����� �� USART */
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		USART_ClearFlag(USART1, USART_IT_RXNE);

		// ������� ����������
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
	//int n = 0;
	bool flag_lvl = check_LEVEL();
	bool flag_rain = check_RAIN();

	if(flag_lvl)
	{
	}
	else
	{
	}

	if(flag_rain)
	{
	}
	else
	{
	}

	switch(state)
	{
	case STATUS_IDLE:
		break;
	case STATUS_RAIN:			// �����
		break;
	case STATUS_WASHOUT:		// ������
		break;
	case STATUS_WASHOUT_PAUSE:	// �����
		break;
	default:
		break;
	}
}
//--------------------------------------------------------------------------------
void command()
{
	int n = 0;
	if(index_buf == 0)
	{
		return;
	}
	if(index_buf % 2)
	{
		return;
	}

	int temp_index_buf = 0;
	for(n=0; n<index_buf; n+=2)
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
		cnt_flash++;

		flag = !flag;
		if(flag)
		{
			blink_ON();
			// �������� 1 ��� � ���.
			work();
		}
		else
		{
			blink_OFF();
		}
	}

#if 0
	if(cnt_flash > 60)
	{
		pump_ON();
	}
#endif
}
//--------------------------------------------------------------------------------
void f_read()
{
	int n = 0;
	union QUESTION_READ *packet = (union QUESTION_READ *)&buf;

	uint16_t crc = crc16((uint8_t *)&buf, sizeof(union QUESTION_READ) - 2);
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

	answer.body.addr_cam_32 = addr_cam_32;                			// ����� ������
	answer.body.time_interval_16 = time_interval_16;           		// �������� ��������
	answer.body.time_washout_32 = time_washout_32;            		// ����� �������
	answer.body.time_pause_washout_32 = time_pause_washout_32;     	// ����� ����� ���������
	answer.body.preset_washout_32 = preset_washout_32;          	// ������ �������
	answer.body.time_preset_washout_32 = time_preset_washout_32;	// ������ �������

	answer.body.crc16 = crc16((uint8_t *)&answer.buf, sizeof(union ANSWER_READ) - 2);

	for(n=0; n<sizeof(answer); n++)
	{
		buf[n] = answer.buf[n];
	}
	index_buf = sizeof(answer);
	send_data();
}
//--------------------------------------------------------------------------------
void f_write()
{
	int n = 0;
	union QUESTION_WRITE *packet = (union QUESTION_WRITE *)&buf;

	uint16_t crc = crc16((uint8_t *)&buf, sizeof(union QUESTION_WRITE) - 2);
	if(crc != packet->body.crc16)
	{
		return;
	}
	uint8_t addr = get_address();
	if(addr != packet->body.header.addr_8)
	{
		return;
	}

	addr_cam_32 = packet->body.addr_cam_32;                			// ����� ������
	time_interval_16 = packet->body.time_interval_16;           	// �������� ��������
	time_washout_32 = packet->body.time_washout_32;            		// ����� �������
	time_pause_washout_32 = packet->body.time_pause_washout_32;     // ����� ����� ���������
	preset_washout_32 = packet->body.preset_washout_32;          	// ������ �������
	time_preset_washout_32 = packet->body.time_preset_washout_32;	// ������ �������

	union ANSWER_WRITE answer;

	answer.body.header.prefix_16 = packet->body.header.prefix_16;
	answer.body.header.addr_8 = packet->body.header.addr_8;
	answer.body.header.cmd_8 = packet->body.header.cmd_8;
	answer.body.header.len_16 = packet->body.header.len_16;

	answer.body.addr_cam_32 = addr_cam_32;                			// ����� ������
	answer.body.time_interval_16 = time_interval_16;           		// �������� ��������
	answer.body.time_washout_32 = time_washout_32;            		// ����� �������
	answer.body.time_pause_washout_32 = time_pause_washout_32;     	// ����� ����� ���������
	answer.body.preset_washout_32 = preset_washout_32;          	// ������ �������
	answer.body.time_preset_washout_32 = time_preset_washout_32;	// ������ �������

	answer.body.crc16 = crc16((uint8_t *)&answer.buf, sizeof(union ANSWER_WRITE) - 2);

	for(n=0; n<sizeof(answer); n++)
	{
		buf[n] = answer.buf[n];
	}
	index_buf = sizeof(answer);
	send_data();
}
//--------------------------------------------------------------------------------
void f_test()
{
	int n = 0;
	union QUESTION_TEST *packet = (union QUESTION_TEST *)&buf;

	uint16_t crc = crc16((uint8_t *)&buf, sizeof(union QUESTION_TEST) - 2);
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

	index_buf = sizeof(answer);
	for(n=0; n<index_buf; n++)
	{
		buf[n] = answer.buf[n];
	}
	send_data();
}
//--------------------------------------------------------------------------------
void f_reset()
{
	int n = 0;
	union QUESTION_RESET *packet = (union QUESTION_RESET *)&buf;

	uint16_t crc = crc16((uint8_t *)&buf, sizeof(union QUESTION_RESET) - 2);
	if(crc != packet->body.crc16)
	{
		return;
	}
	uint8_t addr = get_address();
	if(addr != packet->body.header.addr_8)
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

	for(n=0; n<sizeof(answer); n++)
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
void GPIO_Configuration(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9,  GPIO_AF_1); // USART1 TX
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

	// 485
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// blink
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// relay
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// pump
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// rain
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// level
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);        

	// addr
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 |
			GPIO_Pin_9 |
			GPIO_Pin_10 |
			GPIO_Pin_13 |
			GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
//--------------------------------------------------------------------------------
void USART_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitTypeDef USART_InitStructur;
	USART_InitStructur.USART_BaudRate = 9600;
	USART_InitStructur.USART_WordLength = USART_WordLength_8b;
	USART_InitStructur.USART_StopBits = USART_StopBits_1;
	USART_InitStructur.USART_Parity = USART_Parity_No;
	USART_InitStructur.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructur.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructur);
	USART_Cmd(USART1, ENABLE);

	NVIC_InitTypeDef	NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	// ���������� �� ��������� ������ � �����;
	//USART_ITConfig(USART1, USART_IT_TC, ENABLE);	// ���������� �� ��������� ��������

	NVIC_EnableIRQ (USART1_IRQn);           		// ��������� ���������� �� USART1
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
	int n = 0;
	uint8_t hi_str = 0;
	uint8_t lo_str = 0;

	write_RS485();

	send_byte((char)':');
	for(n=0; n<index_buf; n++)
	{
		convert_data_to_ascii(buf[n], &hi_str, &lo_str);
		send_byte(hi_str);
		send_byte(lo_str);
	}
	send_byte((char)0x0D);

	read_RS485();
}
//--------------------------------------------------------------------------------
void Program(void)
{
	/*!< At this stage the microcontroller clock setting is already configured,
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f0xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f0xx.c file
	 */

	/* Unlock the Flash to enable the flash control register access *************/
	FLASH_Unlock();

	/* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

	/* Define the number of page to be erased */
	NbrOfPage = (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;

	/* Erase the FLASH pages */
	for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
	{
		if (FLASH_ErasePage(FLASH_USER_START_ADDR + (FLASH_PAGE_SIZE * EraseCounter))!= FLASH_COMPLETE)
		{
			/* Error occurred while sector erase.
         User can add here some code to deal with this error  */
			while (1)
			{
			}
		}
	}

	/* Program the user Flash area word by word
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	Address = FLASH_USER_START_ADDR;

	while (Address < FLASH_USER_END_ADDR)
	{
		if (FLASH_ProgramWord(Address, DATA_32) == FLASH_COMPLETE)
		{
			Address = Address + 4;
		}
		else
		{
			/* Error occurred while writing data in Flash memory.
         User can add here some code to deal with this error */
			while (1)
			{
			}
		}
	}

	/* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
	FLASH_Lock();

	/* Check if the programmed data is OK
      MemoryProgramStatus = 0: data programmed correctly
      MemoryProgramStatus != 0: number of words not programmed correctly ******/
	Address = FLASH_USER_START_ADDR;
	MemoryProgramStatus = PASSED;

	while (Address < FLASH_USER_END_ADDR)
	{
		Data = *(__IO uint32_t *)Address;

		if (Data != DATA_32)
		{
			MemoryProgramStatus = FAILED;
		}

		Address = Address + 4;
	}
}
//--------------------------------------------------------------------------------
int main(void)
{
	__enable_irq ();
	SysTick_Config(SystemCoreClock / 1000);

	GPIO_Configuration();
	USART_Configuration();
	read_RS485();

	while(1)
	{
	}
}
//--------------------------------------------------------------------------------
