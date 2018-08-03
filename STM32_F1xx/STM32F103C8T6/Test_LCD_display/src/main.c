//------------------------------------------------------------
//
//------------------------------------------------------------
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
//------------------------------------------------------------
#include "ws2812b.h"
#include "usart.h"
//------------------------------------------------------------
//3.3V 12-255
uint8_t color = 12;
#define NUM_LEDS    6

RGB_t leds[NUM_LEDS];
//------------------------------------------------------------
extern volatile uint8_t Receive_Buffer[200];
extern volatile uint32_t Receive_length ;
extern volatile uint32_t length ;
uint8_t Send_Buffer[64];
uint32_t packet_sent=1;
uint32_t packet_receive=1;
//------------------------------------------------------------
#define MAX_MODBUS_BUF	100
uint8_t modbus_buffer[MAX_MODBUS_BUF];
uint8_t index_modbus_buffer = 0;
//------------------------------------------------------------
char log_buf[1000];
//------------------------------------------------------------
#define SCREEN_WIDTH    32
#define SCREEN_HEIGTH   16
//------------------------------------------------------------
#define CMD_01          0x01
//------------------------------------------------------------
#pragma pack (push, 1)

typedef struct P_HEADER
{
	uint8_t   addr;
	uint8_t   cmd;
	uint16_t  len;
	uint8_t   data[];
} p_header_t;

typedef struct LED
{
	uint8_t color_R;
	uint8_t color_G;
	uint8_t color_B;
} led_t;

typedef struct P_DATA
{
	//uint8_t   	brightness;
	led_t	leds[NUM_LEDS];
	//struct LED	leds[SCREEN_WIDTH][SCREEN_HEIGTH];
} p_data_t;

#pragma pack(pop)
//------------------------------------------------------------
void SetSysClockTo72(void);
void clear_leds(void);
void prepare_red(void);
void prepare_green(void);
void prepare_blue(void);
void init_log_uart();
uint8_t convert(uint8_t x);
uint8_t convert_ascii_to_value(uint8_t hi, uint8_t lo);
bool convert_raw_to_modbus(void);
bool check_packet(void);
//------------------------------------------------------------
void SetSysClockTo72(void)
{
	ErrorStatus HSEStartUpStatus;
	/* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/
	/* RCC system reset(for debug purpose) */
	RCC_DeInit();

	/* Enable HSE */
	RCC_HSEConfig( RCC_HSE_ON);

	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if (HSEStartUpStatus == SUCCESS)
	{
		/* Enable Prefetch Buffer */
		// FLASH_PrefetchBufferCmd( FLASH_PrefetchBuffer_Enable);

		/* Flash 2 wait state */
		// FLASH_SetLatency( FLASH_Latency_2);

		/* HCLK = SYSCLK */
		RCC_HCLKConfig( RCC_SYSCLK_Div1);

		/* PCLK2 = HCLK */
		RCC_PCLK2Config( RCC_HCLK_Div1);

		/* PCLK1 = HCLK/2 */
		RCC_PCLK1Config( RCC_HCLK_Div2);

		/* PLLCLK = 8MHz * 9 = 72 MHz */
		RCC_PLLConfig(0x00010000, RCC_PLLMul_9);

		/* Enable PLL */
		RCC_PLLCmd( ENABLE);

		/* Wait till PLL is ready */
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}

		/* Select PLL as system clock source */
		RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source */
		while (RCC_GetSYSCLKSource() != 0x08)
		{
		}
	}
	else
	{
		/* If HSE fails to start-up, the application will have wrong clock configuration.
     	 User can add here some code to deal with this error */

		/* Go to infinite loop */
		while (1)
		{
		}
	}
}
//------------------------------------------------------------
void clear_leds(void)
{
	int n = 0;

	RGB_t led;
	led.r = 0;
	led.g = 0;
	led.b = 0;
	for(n=0; n<NUM_LEDS; n++)
	{
		leds[n] = led;
	}
}
//------------------------------------------------------------
void prepare_red(void)
{
	int n = 0;

	RGB_t led;
	led.r = color;
	led.g = 0;
	led.b = 0;
	for(n=0; n<NUM_LEDS; n++)
	{
		leds[n] = led;
	}
}
//------------------------------------------------------------
void prepare_green(void)
{
	int n = 0;

	RGB_t led;
	led.r = 0;
	led.g = color;
	led.b = 0;
	for(n=0; n<NUM_LEDS; n++)
	{
		leds[n] = led;
	}
}
//------------------------------------------------------------
void prepare_blue(void)
{
	int n = 0;

	RGB_t led;
	led.r = 0;
	led.g = 0;
	led.b = color;
	for(n=0; n<NUM_LEDS; n++)
	{
		leds[n] = led;
	}
}
//------------------------------------------------------------
void init_log_uart()
{
	usart_init();
	usart_put_str("usart_init\n");
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
#if 0
String convert_data(uint8_t x)
{
	String res;
	switch (x)
	{
	case 0x00: res = "0"; break;
	case 0x01: res = "1"; break;
	case 0x02: res = "2"; break;
	case 0x03: res = "3"; break;
	case 0x04: res = "4"; break;
	case 0x05: res = "5"; break;
	case 0x06: res = "6"; break;
	case 0x07: res = "7"; break;
	case 0x08: res = "8"; break;
	case 0x09: res = "9"; break;
	case 0x0A: res = "A"; break;
	case 0x0B: res = "B"; break;
	case 0x0C: res = "C"; break;
	case 0x0D: res = "D"; break;
	case 0x0E: res = "E"; break;
	case 0x0F: res = "F"; break;
	default:
		break;
	}
	return res;
}
#endif
//------------------------------------------------------------
uint8_t convert_ascii_to_value(uint8_t hi, uint8_t lo)
{
	uint8_t b_hi = convert(hi);
	uint8_t b_lo = convert(lo);

	uint8_t r_byte = (b_hi << 4) | b_lo;
	return r_byte;
}
//------------------------------------------------------------
bool convert_raw_to_modbus(void)
{
	if(Receive_length == 0)
	{
		usart_put_str("empty data\n");
		return FALSE;
	}
	if(Receive_Buffer[0] != ':')
	{
		usart_put_str("bad Receive_Buffer[0]\n");
		return FALSE;
	}
	if(Receive_Buffer[Receive_length - 1] != '\n')
	{
		usart_put_str("bad Receive_Buffer[Receive_length]\n");
		return FALSE;
	}
	if((Receive_length % 2) != 0)
	{
		usart_put_str("bad (Receive_length % 2)\n");
		return FALSE;
	}

	index_modbus_buffer = 0;
	int n = 0;
	for (n = 1; n < Receive_length; n += 2)
	{
		modbus_buffer[index_modbus_buffer] = convert_ascii_to_value(Receive_Buffer[n], Receive_Buffer[n + 1]);
		index_modbus_buffer++;
		if (index_modbus_buffer > MAX_MODBUS_BUF)
		{
			usart_put_str("packet too large\m");
			return FALSE;
		}
	}
	if (index_modbus_buffer < sizeof(p_header_t))
	{
		usart_put_str("packet too small\m");
		return FALSE;
	}

	return TRUE;
}
//------------------------------------------------------------
bool check_packet(void)
{
	usart_put_str("check_packet\n");
	usart_put_str(Receive_Buffer);

	bool ok = convert_raw_to_modbus();
	if(ok == FALSE)
	{
		usart_put_str("bad convert\n");
		return FALSE;
	}

	if ((index_modbus_buffer - 1)  != (sizeof(p_header_t) + sizeof(p_data_t)))
	{
		usart_put_str("bad size\n");

		sprintf(log_buf, "index %d\n", index_modbus_buffer);
		usart_put_str(log_buf);

		sprintf(log_buf, "need %d\n", (sizeof(p_header_t) + sizeof(p_data_t)));
		usart_put_str(log_buf);
		return FALSE;
	}

	p_header_t *header = (p_header_t *)&modbus_buffer;
	p_data_t   *data   = (p_data_t *)&header->data;

	int n = 0;

	RGB_t led;

	for(n=0; n<NUM_LEDS; n++)
	{
		led.r = data->leds[n].color_R;
		led.g = data->leds[n].color_G;
		led.b = data->leds[n].color_B;
		leds[n] = led;

		sprintf(log_buf, "n RGB %d %d %d %d\n", n, led.r, led.g, led.b);
		usart_put_str(log_buf);
	}

	while (!ws2812b_IsReady()); // wait
	ws2812b_SendRGB(leds, NUM_LEDS);

	return TRUE;
}
//------------------------------------------------------------
int main(void)
{
	Set_System();
	SetSysClockTo72();
	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();

	init_log_uart();

	//---
	ws2812b_Init();
	while (!ws2812b_IsReady()); // wait
	clear_leds();
	ws2812b_SendRGB(leds, NUM_LEDS);
	//---

	while (1)
	{
		if (bDeviceState == CONFIGURED)
		{
			CDC_Receive_DATA();
			// Check to see if we have data yet
			if (Receive_length  != 0)
			{
				bool res = check_packet();
				if(res == FALSE)
				{
					usart_put_str("check_packet return FALSE\n");
				}
				Receive_length = 0;
			}
		}
	}
}
//------------------------------------------------------------
