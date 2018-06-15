
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#include "ws2812b.h"

//3.3V 12-255
uint8_t color = 12;
#define NUM_LEDS    30

RGB_t leds[NUM_LEDS];

void Delay_ms(uint32_t ms)
{
	volatile uint32_t nCount;
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq (&RCC_Clocks);

	nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
	for (; nCount!=0; nCount--);
}

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

int main(void)
{
#if 1
	char flag = 0;
	while(1)
	{
		flag = !flag;
		if(flag)
		{
			ws2812b_Init2(GPIOB, GPIO_Pin_6);
			while (!ws2812b_IsReady()); // wait
			prepare_red();
			ws2812b_SendRGB(leds, NUM_LEDS);
		}
		else
		{
			ws2812b_Init2(GPIOB, GPIO_Pin_8);
			while (!ws2812b_IsReady()); // wait
			prepare_blue();
			ws2812b_SendRGB(leds, NUM_LEDS);
		}
		Delay_ms(1000);
	}
#else
	ws2812b_Init();
	int n = 0;

	while (1)
	{
		while (!ws2812b_IsReady()); // wait
		//---
		switch(n)
		{
		case 0:
			prepare_red();
			break;
		case 1:
			prepare_green();
			break;
		case 2:
			prepare_blue();
			break;
		default:
			break;
		}
		if(n<2)
			n++;
		else
			n=0;
		//---
		ws2812b_SendRGB(leds, NUM_LEDS);
		Delay_ms(333);
	}
#endif
}