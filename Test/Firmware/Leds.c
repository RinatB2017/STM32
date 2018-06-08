#include "Leds.h"

volatile Led_Status LD_ST[4];
volatile uint8_t LD_CNT[4];
volatile uint8_t LD_CNT_BLK[4];
volatile uint8_t LD_CNT_ST[4];

__inline void StatusLeds_LED_ON(uint8_t led)
{
	if(led == 0) GPIO_SetBits(STATUSLED_LINK_PORT, STATUSLED_LINK_GREEN_PIN);
	else if(led == 1) GPIO_SetBits(STATUSLED_LINK_PORT, STATUSLED_LINK_RED_PIN);
	else if(led == 2) GPIO_SetBits(STATUSLED_LINK_PORT, STATUSLED_WORK_GREEN_PIN);
	else if(led == 3) GPIO_SetBits(STATUSLED_LINK_PORT, STATUSLED_WORK_RED_PIN);	
}

__inline void StatusLeds_LED_OFF(uint8_t led)
{
	if(led == 0) GPIO_ResetBits(STATUSLED_LINK_PORT, STATUSLED_LINK_GREEN_PIN);
	else if(led == 1) GPIO_ResetBits(STATUSLED_LINK_PORT, STATUSLED_LINK_RED_PIN);
	else if(led == 2) GPIO_ResetBits(STATUSLED_LINK_PORT, STATUSLED_WORK_GREEN_PIN);
	else if(led == 3) GPIO_ResetBits(STATUSLED_LINK_PORT, STATUSLED_WORK_RED_PIN);	
}

void StatusLeds_Init(void)
{
	GPIO_InitTypeDef PORT;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
	
	// Link Leds
	PORT.GPIO_Speed = GPIO_Speed_10MHz;
	PORT.GPIO_Pin = (STATUSLED_LINK_GREEN_PIN | STATUSLED_LINK_RED_PIN);
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(STATUSLED_LINK_PORT, &PORT);
	
	// Work Leds
	PORT.GPIO_Pin = (STATUSLED_WORK_GREEN_PIN | STATUSLED_WORK_RED_PIN);
	GPIO_Init(STATUSLED_WORK_PORT, &PORT);
}


void StatusLeds_Set(Leds_Select led, Led_Status status)
{
	if((uint8_t)led < 4)
	{
		LD_ST[led] = status;
		
		if((status == Led_Blink_Fast) || (status == Led_Blink_Slow))
		{
			LD_CNT[led] = 0;
			LD_CNT_ST[led] = 0;
		}
	}
}

void StatusLeds_Blink(Leds_Select led)
{
	if((uint8_t)led < 4)
	{
		LD_CNT_BLK[led] = STATUSLED_BLINK_LINK_CNT;
	}
}

void StatusLeds_Routine(void)
{
	uint8_t i = 0;
	for(i=0;i<4;i++)
	{
		if(LD_CNT_BLK[i])
		{
			if(LD_CNT_BLK[i] > (STATUSLED_BLINK_LINK_CNT / 2))
			{
				StatusLeds_LED_ON(i);
			}
			else
			{
				StatusLeds_LED_OFF(i);
			}
			
			LD_CNT_BLK[i]--;
		}
		else
		{
			switch(LD_ST[i])
			{
				case Led_OFF:
					StatusLeds_LED_OFF(i);
				break;
				
				case Led_ON:
					StatusLeds_LED_ON(i);
				break;
				
				case Led_Blink_Fast:
					if(LD_CNT[i] < STATUSLED_BLINK_FAST_CNT)
					{
						LD_CNT[i]++;
					}
					else
					{
						LD_CNT[i]=0;
						if(LD_CNT_ST[i])
						{
							StatusLeds_LED_OFF(i);
						}
						else
						{
							StatusLeds_LED_ON(i);
						}
						LD_CNT_ST[i] = !LD_CNT_ST[i];
					}
				break;
				
				case Led_Blink_Slow:
					if(LD_CNT[i] < STATUSLED_BLINK_SLOW_CNT)
					{
						LD_CNT[i]++;
					}
					else
					{
						LD_CNT[i]=0;
						if(LD_CNT_ST[i])
						{
							StatusLeds_LED_OFF(i);
						}
						else
						{
							StatusLeds_LED_ON(i);
						}
						LD_CNT_ST[i] = !LD_CNT_ST[i];
					}
				break;
			}
		}
	}
}
