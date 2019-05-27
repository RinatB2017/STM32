/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"
#include <nucleo_hal_bsp.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim1;

/* Frequency Value */
float frequency = 0;
void MX_TIM1_Init(void);
void MX_TIM6_Init(void);
void MX_DMA_Init(void);

DMA_HandleTypeDef hdma_tim1_ch1;
DMA_HandleTypeDef hdma_tim6_up;

uint8_t  odrVals[] = { 0x0, 0xFF };
uint16_t captures[2];
volatile uint8_t  captureDone = 0;

int main(void) {
  uint16_t diffCapture = 0;
  char msg[30];

  HAL_Init();

  Nucleo_BSP_Init();
  MX_DMA_Init();

  MX_TIM1_Init();
  MX_TIM6_Init();

  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_IC_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t*) captures, 2);

  while (1) {
    if (captureDone != 0) {
      if (captures[1] >= captures[0])
        diffCapture = captures[1] - captures[0];
      else
        diffCapture = (htim1.Instance->ARR - captures[0]) + captures[1];

      frequency = HAL_RCC_GetHCLKFreq() / (htim1.Instance->PSC + 1);
      frequency = (float) frequency / diffCapture;

      sprintf(msg, "Input frequency: %.3f\r\n", frequency);
      HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);
      while (1);
    }
  }
}

/* TIM1 init function */
void MX_TIM1_Init(void) {
  TIM_IC_InitTypeDef sConfigIC;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_IC_Init(&htim1);

  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_1);
}

void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* htim_ic) {
  GPIO_InitTypeDef GPIO_InitStruct;
  if (htim_ic->Instance == TIM1) {
    /* Peripheral clock enable */
    __TIM1_CLK_ENABLE();

    /**TIM3 GPIO Configuration
     PC0     ------> TIM1_CH1
     */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Peripheral DMA init*/
    hdma_tim1_ch1.Instance = DMA1_Channel2;
    hdma_tim1_ch1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_tim1_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim1_ch1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim1_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_tim1_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_tim1_ch1.Init.Mode = DMA_NORMAL;
    hdma_tim1_ch1.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_tim1_ch1);

    __HAL_LINKDMA(htim_ic,hdma[TIM_DMA_ID_CC1],hdma_tim1_ch1);
  }
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
    captureDone = 1;
  }
}

void MX_TIM6_Init(void) {
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 319; //64MHz/320 = 200KHz
  htim6.Init.Period = 1;      //200KHz/2  = 100KHz
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  HAL_TIM_Base_Init(&htim6);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base) {
  if (htim_base->Instance == TIM6) {
    /* Peripheral clock enable */
    __TIM6_CLK_ENABLE();

    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  }
}

void MX_DMA_Init(void) {
  /* DMA controller clock enable */
  __DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

}

void DMA1_Channel2_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_tim1_ch1);
}

void TIM6_DAC_IRQHandler(void) {
  HAL_TIM_IRQHandler(&htim6);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if(htim->Instance == TIM6)
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}


#ifdef USE_FULL_ASSERT

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
