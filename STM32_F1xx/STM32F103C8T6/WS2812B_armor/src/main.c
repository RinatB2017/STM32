
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#include "ws2812b.h"

//3.3V 12-255
uint8_t color = 50;
#define NUM_LEDS    6

RGB_t leds_1[NUM_LEDS];
RGB_t leds_2[NUM_LEDS];

void Delay_ms(uint32_t ms)
{
	volatile uint32_t nCount;
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq (&RCC_Clocks);

	nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
	for (; nCount!=0; nCount--);
}

void prepare_red_1(void)
{
	int n = 0;

	RGB_t led;
	led.r = color;
	led.g = 0;
	led.b = 0;
	for(n=0; n<NUM_LEDS; n++)
	{
		leds_1[n] = led;
	}
}

void prepare_red_2(void)
{
	int n = 0;

	RGB_t led;
	led.r = color;
	led.g = 0;
	led.b = 0;
	for(n=0; n<NUM_LEDS; n++)
	{
		leds_2[n] = led;
	}
}

void prepare_green_1(void)
{
	int n = 0;

	RGB_t led;
	led.r = 0;
	led.g = color;
	led.b = 0;
	for(n=0; n<NUM_LEDS; n++)
	{
		leds_1[n] = led;
	}
}

void prepare_green_2(void)
{
	int n = 0;

	RGB_t led;
	led.r = 0;
	led.g = color;
	led.b = 0;
	for(n=0; n<NUM_LEDS; n++)
	{
		leds_2[n] = led;
	}
}

void prepare_blue_1(void)
{
	int n = 0;

	RGB_t led;
	led.r = 0;
	led.g = 0;
	led.b = color;
	for(n=0; n<NUM_LEDS; n++)
	{
		leds_1[n] = led;
	}
}

void prepare_blue_2(void)
{
	int n = 0;

	RGB_t led;
	led.r = 0;
	led.g = 0;
	led.b = color;
	for(n=0; n<NUM_LEDS; n++)
	{
		leds_2[n] = led;
	}
}

int main(void)
{
	//ws2812b_Init_1();
	//ws2812b_Init_2();
	ws2812b_Init_4();

	while (1)
	{
#if 0
		while (!ws2812b_IsReady_1());
		prepare_red_1();
		ws2812b_SendRGB_1(leds_1, NUM_LEDS);
#endif

		//while (!ws2812b_IsReady_2());
		//prepare_blue_2();
		//ws2812b_SendRGB_2(leds_2, NUM_LEDS);

#if 1
		while (!ws2812b_IsReady_1());
		prepare_blue_1();
		ws2812b_SendRGB_1(leds_1, NUM_LEDS);
#endif
	}
}
