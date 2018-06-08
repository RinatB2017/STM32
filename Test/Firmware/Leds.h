#ifndef _LEDS_IM_H_
#define _LEDS_IM_H_

#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>

typedef enum
{
	Led_OFF = 0,
	Led_ON,
	Led_Blink_Fast,
	Led_Blink_Slow
} Led_Status;

typedef enum 
{
	Led_Link_Green = 0,
	Led_Link_Red,
	Led_Work_Green,
	Led_Work_Red
} Leds_Select;

#ifndef STATUSLED_LINK_PORT
#define STATUSLED_LINK_PORT				GPIOB
#define STATUSLED_LINK_GREEN_PIN	GPIO_Pin_3
#define STATUSLED_LINK_RED_PIN		GPIO_Pin_8
#endif

#ifndef STATUSLED_WORK_PORT
#define STATUSLED_WORK_PORT				GPIOB
#define STATUSLED_WORK_GREEN_PIN	GPIO_Pin_4
#define STATUSLED_WORK_RED_PIN		GPIO_Pin_9
#endif

#define STATUSLED_BLINK_FAST_CNT	15
#define STATUSLED_BLINK_SLOW_CNT	30
#define STATUSLED_BLINK_LINK_CNT	20

void StatusLeds_Init(void);
void StatusLeds_Set(Leds_Select led, Led_Status status);
void StatusLeds_Blink(Leds_Select led);
void StatusLeds_Routine(void);
#endif // _LEDS_IM_H_
