
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#include "ws2812b.h"

#define NUM_LEDS    6

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
    // ��������� RGB-������
    RGB_t led;
    led.r = 0xFF;
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
    // ��������� RGB-������
    RGB_t led;
    led.r = 0;
    led.g = 0xFF;
    led.b = 0;
    for(n=0; n<NUM_LEDS; n++)
    {
    	leds[n] = led;
    }
}

void prepare_blue(void)
{
	int n = 0;
    // ��������� RGB-������
    RGB_t led;
    led.r = 0;
    led.g = 0;
    led.b = 0xFF;
    for(n=0; n<NUM_LEDS; n++)
    {
    	leds[n] = led;
    }
}

int main(void)
{
#if 0
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

        while(1)
        {
                GPIO_SetBits(GPIOB,	GPIO_Pin_12);
                Delay_ms(500);
                GPIO_ResetBits(GPIOB,	GPIO_Pin_12);
                Delay_ms(500);
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

                Delay_ms(500);
        }
#endif
}
