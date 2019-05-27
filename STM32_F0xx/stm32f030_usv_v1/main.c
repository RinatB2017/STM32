#include <stdint.h>
#include <system_stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <rcc.h>
#include <gpio.h>
#include <usart.h>
#include <FLASH.h>
#include <EXTI.h>
#include <TIMERS.h>
#include <io_proc.h>
#include <RFI.h>
//#include <ADC.h>
#include <mif.h>

//################################
char in_cmd=0;				//	���������� ������� ������� ���
char command=0;				//	���������� �������� ������� ���
char Buffer_in [0x500];		// 	������ ��������
short lench_Buffer_in =0 ;	// 	������ �������� � Buffer_in !!!

void SysTick_Handler(void)	// ���������� SysTick
{
	mif_SysTick_Handler();
}

int main(void)
{
	init_mif();

	//---
	/*
	while(1)
	{
		output_485();
		for(char n=0; n<10; n++)
		{
			uart2(n);
		}
//		uart2((char)'H');
//		uart2((char)'e');
//		uart2((char)'l');
//		uart2((char)'l');
//		uart2((char)'o');
		uart2(0x0D);
		input_485();

		Delay_ms(1000);
	}
	*/
	//---

    while(1)
    {
    	//################################// ��������� ������ 	################################
		if	(Buffer_in[2])
		{
			in_cmd = Buffer_in[2];
			Buffer_in[2]=0;
		}
		if	((in_cmd != 0) && (command == 0))	// ��������� ������� �� 485
		{
			command = in_cmd;
			in_cmd = 0;
		}
		if  ((in_cmd != 0) && (command != 0) && (Buffer_in[2] !=0 ))
		{
			uart1_32(0x0007BBAA);
			uart1_32(0xEEEEEEEE);
			uart1(command);
			uart1(in_cmd);
			uart1(Buffer_in[2]);
			command = 0;
		}	//������ ��������� ������� ds ERRoR
		command = mif_command(command);	// ��������� ������
    }
}
