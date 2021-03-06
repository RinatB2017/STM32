/**
  *****************************************************************************
  * @title   main.c
  * @author  Mazen21
  * @date    14 Oct 2012
  * @brief
  *******************************************************************************
  */
////// The above comment is automatically generated by CoIDE ///////////////////

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <CoOs.h>

#define STACK_SIZE_DEFAULT 512

OS_STK task1_stk[STACK_SIZE_DEFAULT];
OS_STK task2_stk[STACK_SIZE_DEFAULT];

void initializeBoard()
{
	GPIO_InitTypeDef GPIO_InitStructure_Led;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

	GPIO_InitStructure_Led.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure_Led.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure_Led.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOB,&GPIO_InitStructure_Led);
}

void task1 (void* pdata)
{
	while(1)
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET);
		CoTimeDelay (0, 0, 0, 300);
		GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);
		CoTimeDelay (0, 0, 0, 300);
	}
}

void task2 (void* pdata)
{
	while(1)
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_SET);
		CoTimeDelay (0, 0, 0, 400);
		GPIO_WriteBit(GPIOB, GPIO_Pin_6 ,Bit_RESET);
		CoTimeDelay (0, 0, 0, 400);
	}
}

int main(void)
{
	initializeBoard();
	CoInitOS();

	CoCreateTask(task1,0,0,&task1_stk[STACK_SIZE_DEFAULT-1],STACK_SIZE_DEFAULT);
	CoCreateTask(task2,0,1,&task2_stk[STACK_SIZE_DEFAULT-1],STACK_SIZE_DEFAULT);
	CoStartOS();
	while(1);
}
