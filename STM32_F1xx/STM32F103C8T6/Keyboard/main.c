
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

char keyboard[4][4] = { {0, 0} };

int main(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	int state = 0;
	while(1)
	{
		if(state > 3) state = 0;
		else state++;

		GPIO_Write(GPIOA, 1 << state);
		keyboard[state][0] = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4);
		keyboard[state][1] = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);
		keyboard[state][2] = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6);
		keyboard[state][3] = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7);

		if(keyboard[1][1] != 0)
		{
			GPIO_SetBits(GPIOB,	GPIO_Pin_7);
		}
		else
		{
			GPIO_ResetBits(GPIOB,	GPIO_Pin_7);
		}
	}
}
