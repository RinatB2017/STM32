#include <stdint.h>
#include <system_stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>

void RCC_Configuration_HSE48(void)
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RCC_Configuration_HSI48(void)
{
	uint16_t	HSEStartUpStatus;

    /*RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
//    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
//    HSEStartUpStatus = RCC_WaitForHSEStartUp();

//    if (HSEStartUpStatus == SUCCESS)
//    {
    //	 FLASH_SetLatency(FLASH_Latency_2);
        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        /* PCLK2 = HCLK*/
  //      RCC_PCLK2Config(RCC_HCLK_Div1);

        /* PCLK1 = HCLK*/
  //      RCC_PCLK1Config(RCC_HCLK_Div1);

        //ADC CLK
//        RCC_ADCCLKConfig(RCC_PCLK2_Div2);

        /* PLLCLK = 8MHz /2 * 12 = 48 MHz */
//-------------------------------------------------------------------------------------------------
//#define RCC_PLLSource_HSI_Div2           RCC_CFGR_PLLSRC_HSI_Div2
//#define RCC_PLLSource_PREDIV1            RCC_CFGR_PLLSRC_PREDIV1
//-------------------------------------------------------------------------------------------------
        RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_12);//,RCC_PLLSource_PREDIV1, RCC_PLLMul_12);

        /* Enable PLL */
        RCC_PLLCmd(ENABLE);

        /* Wait till PLL is ready */
//        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}

        /* Select PLL as system clock source */
//---------------------------------------------------------------------------------------------------
//#define RCC_SYSCLKSource_HSI             RCC_CFGR_SW_HSI
//#define RCC_SYSCLKSource_HSE             RCC_CFGR_SW_HSE
//#define RCC_SYSCLKSource_PLLCLK          RCC_CFGR_SW_PLL
//#define IS_RCC_SYSCLK_SOURCE(SOURCE) (((SOURCE) == RCC_SYSCLKSource_HSI) || \
//                                     ((SOURCE) == RCC_SYSCLKSource_HSE) || \
//                                      ((SOURCE) == RCC_SYSCLKSource_PLLCLK))
//----------------------------------------------------------------------------------------------------
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);//RCC_CFGR_SW_HSI);//RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
//        while (RCC_GetSYSCLKSource() != 0x08) {}
//    }

    /*Then need to enable peripheral clocks ----------------------------------------------*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Delay_ms(uint32_t ms)
{
	volatile uint32_t nCount;
	RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq (&RCC_Clocks);

	nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
	for (; nCount!=0; nCount--);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
