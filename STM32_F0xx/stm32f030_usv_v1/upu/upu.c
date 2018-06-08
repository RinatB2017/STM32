


#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"

#include "defines.h"

long	upu_time_tmp = 0x400;	// ¬–≈Ã≈ÕÕ¿ﬂ œ≈–≈Ã≈ÕÕ¿ﬂ
long	upu_time = 0x4F5;		// 8MhZ == 500 mSec//  ƒ»— –≈“ÕŒ—“‹
char	flag = 0;

void upu_RCC_Configuration_HSE48(void)
{
	uint16_t	HSEStartUpStatus;

    /*RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if (HSEStartUpStatus == SUCCESS)
    {
    //	 FLASH_SetLatency(FLASH_Latency_2);
        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        /* PCLK2 = HCLK*/
  //      RCC_PCLK2Config(RCC_HCLK_Div1);

        /* PCLK1 = HCLK*/
  //      RCC_PCLK1Config(RCC_HCLK_Div1);

        //ADC CLK
//        RCC_ADCCLKConfig(RCC_PCLK2_Div2);

        /* PLLCLK = 8MHz * 6 = 48 MHz */
        RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_6);

        /* Enable PLL */
        RCC_PLLCmd(ENABLE);

        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x08) {}
    }

    /*Then need to enable peripheral clocks ----------------------------------------------*/
}

void upu_SysTick_Handler(void)
{
	if	(upu_time_tmp-- == 0)
	{
		upu_time_tmp = upu_time;  //// ƒ≈ –≈Ã≈Õ“ ¬–≈Ã≈Õ»

		flag = !flag;
		if(flag) {
			GPIO_SetBits(GPIOB, GPIO_Pin_1);
		}
		else {
			GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		}
	}
}

void init_timer(void)
{
	upu_RCC_Configuration_HSE48();
	SysTick_Config(APBCLK / 5000);	//	100 ÏkÒ.//SysTick_Config(SystemFrequency / 1000);	// Õ‡ÒÚÓÈÍ‡ SysTick - 1 ÏÒ.
}

void led_on(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
}

void led_off(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_1);
}

void init(void)
{
    // Enable GPIO clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    // Configure pin as output push-pull (LED)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //led_on();
    GPIO_SetBits(GPIOB, GPIO_Pin_1);

    //init_timer();
}
