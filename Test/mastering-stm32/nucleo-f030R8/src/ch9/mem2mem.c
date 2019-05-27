/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include <nucleo_hal_bsp.h>
#include <string.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
extern UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_memtomem_dma1_channel5;


/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

const uint8_t flashData[] = {0xe7, 0x49, 0x9b, 0xdb, 0x30, 0x5a, 0xc3, 0xaf, 0xbc, 0xa5, 0xf, 0x62, 0x6f, 0xd2, 0xc0, 0x28, 0xc0, 0x97, 0xb5, 0xb1, 0xc8, 0xda, 0xf1, 0xc8, 0x36, 0x18, 0xbc, 0x23, 0x70, 0x6, 0x73, 0xc, 0xc5, 0x45, 0x9b, 0x6e, 0xdb, 0xc3, 0xd7, 0xac, 0xeb, 0xb4, 0x72, 0xdd, 0xb8, 0x33, 0x9f, 0x5b, 0x1, 0x7c, 0x50, 0x7, 0x7d, 0x49, 0x7a, 0xc0, 0x8d, 0xc3, 0x50, 0xe, 0xe4, 0x4d, 0x31, 0x22, 0x25, 0xee, 0x79, 0xbc, 0xe9, 0x59, 0x1c, 0xb8, 0x86, 0xac, 0x34, 0xe, 0xa3, 0xe0, 0x5c, 0x4a, 0x7d, 0xd0, 0x8e, 0x5c, 0x18, 0xbc, 0x36, 0x5c, 0x4b, 0xfd, 0xfa, 0x8c, 0xaf, 0x4c, 0x9b, 0x83, 0x37, 0xf7, 0x73, 0x16, 0xeb, 0x43, 0x7c, 0xb4, 0x83, 0x7a, 0x31, 0xf1, 0xd6, 0xa2, 0x5f, 0x3, 0xa7, 0x5f, 0x3e, 0x96, 0x28, 0x9a, 0x11, 0xa6, 0x9d, 0x76, 0xc4, 0x40, 0x3e, 0xc0, 0x14, 0xf9, 0x42, 0xbe, 0x12, 0x3a, 0x35, 0x24, 0x9c, 0xd9, 0x4c, 0xb1, 0x8, 0x55, 0x7f, 0xb4, 0xff, 0x88, 0x5f, 0x8, 0x9e, 0x7c, 0x4a, 0x44, 0x1e, 0x5f, 0xdc, 0x9d, 0x36, 0x27, 0xc7, 0x4d, 0xf5, 0xac, 0x9e, 0xf6, 0x19, 0xef, 0x32, 0x5c, 0x15, 0x27, 0x6f, 0x19, 0x22, 0xe3, 0xc5, 0x48, 0xa4, 0x7a, 0x8c, 0xaf, 0x9e, 0x99, 0x3c, 0x6a, 0xa1, 0x5a, 0x9a, 0xee, 0x2f, 0x6b, 0x43, 0x33, 0x9f, 0x9, 0x9a, 0x63, 0x7e, 0x83, 0xb2, 0x55, 0x25, 0x77, 0x6e, 0x20, 0x12, 0x81, 0xeb, 0xbe, 0x67, 0x43, 0x1f, 0xb6, 0xd3, 0x8f, 0x5a, 0x50, 0xcc, 0x6b, 0xce, 0xf6, 0x1a, 0xe6, 0xda, 0x8a, 0x66, 0x42, 0xa1, 0xfd, 0x53, 0x59, 0xef, 0x29, 0xcf, 0xb7, 0x2c, 0x19, 0x73, 0x3f, 0x28, 0xeb, 0x55, 0xee, 0x7b, 0xcb, 0x39, 0x59, 0xc1, 0x2d, 0x39, 0x34, 0xea, 0x5e, 0xb0, 0x25, 0x5d, 0x6b, 0xa4, 0x5d, 0x91, 0x3c, 0x2f, 0xd1, 0x6d, 0x15, 0x47, 0x20, 0xbc, 0x63, 0x92, 0x3f, 0x36, 0x12, 0xa, 0x56, 0x4b, 0xa2, 0x64, 0x33, 0x14, 0xd8, 0x4b, 0x3a, 0x98, 0x23, 0x26, 0x6a, 0x96, 0x9d, 0x58, 0xc6, 0x46, 0xb0, 0xbe, 0xd8, 0x18, 0xc0, 0x16, 0xcc, 0xef, 0xf4, 0x3c, 0x97, 0x83, 0xdb, 0x47, 0x1e, 0xd2, 0x91, 0x61, 0xce, 0x32, 0x2d, 0x1, 0xb5, 0x50, 0xbe, 0xc6, 0xde, 0xfc, 0xbe, 0x3, 0xa6, 0x84, 0x64, 0x41, 0x7, 0xae, 0x77, 0x1, 0x4a, 0x96, 0xcb, 0xf0, 0x22, 0xe5, 0x84, 0xe3, 0xfb, 0xa, 0x78, 0x78, 0x15, 0x37, 0xbf, 0xf5, 0x90, 0x98, 0xd1, 0xa4, 0xe9, 0xa0, 0x59, 0xbe, 0x6f, 0x7b, 0xf3, 0x7c, 0x81, 0x6f, 0x7e, 0x97, 0x9f, 0x5, 0xf7, 0x7e, 0xa0, 0x67, 0xb4, 0x2d, 0x28, 0xfd, 0x90, 0xc, 0x1d, 0xe4, 0xaa, 0x9f, 0x6e, 0xb6, 0x96, 0x8d, 0x81, 0x27, 0x77, 0xbb, 0x48, 0xca, 0xf0, 0xc4, 0x8d, 0xd5, 0x7, 0xd5, 0xbc, 0x54, 0xe5, 0x9f, 0xd8, 0x55, 0x52, 0x2f, 0x8f, 0x2e, 0x45, 0x40, 0x12, 0xc3, 0x19, 0xe9, 0x69, 0x2a, 0x1a, 0x4b, 0x81, 0xd7, 0x46, 0xbf, 0xf6, 0x25, 0x91, 0xb7, 0x6d, 0xa0, 0x7, 0x69, 0x99, 0xc1, 0x20, 0x1a, 0xb, 0x4e, 0xb5, 0x20, 0x5e, 0x60, 0x14, 0x5b, 0x91, 0x1e, 0xde, 0x5d, 0x17, 0xe1, 0x1e, 0xbf, 0x90, 0x2c, 0xaa, 0xbe, 0x5b, 0xc3, 0x57, 0xc6, 0x99, 0x28, 0x6e, 0xd, 0x2f, 0x75, 0x61, 0xcd, 0xe4, 0xea, 0x84, 0xe8, 0x8e, 0x85, 0x14, 0x30, 0x64, 0x5, 0xb8, 0x3b, 0x8d, 0x1d, 0x85, 0x5e, 0xa, 0xe4, 0x69, 0x4a, 0x85, 0xc4, 0xb7, 0x4e, 0x1e, 0xa8, 0xd0, 0x8a, 0x76, 0xdc, 0x8e, 0xd9, 0xc8, 0x57, 0x46, 0xa0, 0x86, 0x7d, 0xb6, 0x21, 0x4c, 0x95, 0x69, 0xaf, 0x8, 0x33, 0x3d, 0x1c, 0x69, 0x93, 0xd9, 0x26, 0xd9, 0xfe, 0x60, 0x6a, 0x13, 0xec, 0x37, 0x96, 0xb0, 0xcb, 0x56, 0xaf, 0x9, 0x65, 0xc3, 0x74, 0x98, 0x83, 0x74, 0x2e, 0x26, 0x3b, 0xef, 0xd8, 0xb8, 0xdb, 0xa9, 0x1a, 0x91, 0x1b, 0x7d, 0xfe, 0x55, 0xcb, 0x6d, 0x95, 0x9f, 0xa0, 0x24, 0xcb, 0x6a, 0xad, 0x2d, 0x80, 0xb2, 0x71, 0xb7, 0x68, 0x8, 0xa, 0xa2, 0x8b, 0xc2, 0xf1, 0x4d, 0xee, 0x10, 0xea, 0x29, 0x36, 0x87, 0x6d, 0x19, 0x3c, 0x65, 0x91, 0xc7, 0x97, 0xa6, 0xa7, 0xba, 0xee, 0x9f, 0x1f, 0x9f, 0x23, 0x86, 0xa0, 0x41, 0xe5, 0x7e, 0x5, 0x5d, 0x44, 0x66, 0xc9, 0x3f, 0xc1, 0x6f, 0x82, 0xc3, 0x92, 0xcb, 0xae, 0xa5, 0x16, 0xf2, 0xe8, 0xef, 0xe2, 0xca, 0x57, 0x9, 0x72, 0xfb, 0x5d, 0x5c, 0x7e, 0xb2, 0xe5, 0x8f, 0x62, 0x52, 0x17, 0x1a, 0xa1, 0xad, 0xd9, 0xa0, 0xef, 0xb1, 0x60, 0x17, 0xd3, 0x21, 0x6, 0x25, 0x18, 0xa8, 0xfc, 0xc5, 0x4, 0x73, 0xac, 0x77, 0xa9, 0xcf, 0x83, 0xca, 0xd, 0x4e, 0x29, 0xa6, 0x3, 0xbb, 0xdb, 0xb9, 0xae, 0x8c, 0x71, 0xcd, 0xfe, 0x7a, 0xb4, 0x37, 0xd, 0xd, 0xc, 0x6a, 0x2a, 0x28, 0xf2, 0x97, 0xf7, 0x1b, 0xf0, 0xe4, 0x8d, 0xb6, 0xcb, 0x8d, 0x2, 0xcb, 0xe6, 0x60, 0xa4, 0x9, 0xe3, 0x7b, 0x27, 0xb, 0xa7, 0x92, 0x66, 0x89, 0xc7, 0xb8, 0xa6, 0xa9, 0x54, 0xb3, 0x85, 0x97, 0x5f, 0x87, 0x8e, 0xb4, 0x11, 0x3c, 0xd1, 0x4, 0x38, 0x35, 0xf5, 0xc2, 0x9f, 0xa5, 0x55, 0x92, 0xf9, 0xa8, 0x83, 0x24, 0x63, 0xb, 0xb5, 0x17, 0x90, 0x15, 0xa9, 0xff, 0x4a, 0xba, 0xe2, 0x77, 0xd4, 0xfa, 0x78, 0xe2, 0xe4, 0x2a, 0x32, 0xab, 0xd1, 0x61, 0x6a, 0x4d, 0x21, 0x86, 0x3b, 0x99, 0xb9, 0x59, 0x13, 0xc5, 0x53, 0x7, 0x8f, 0xd8, 0xfb, 0x77, 0xad, 0x9c, 0x2e, 0xe8, 0x1f, 0x94, 0x87, 0x57, 0x51, 0x1e, 0xaf, 0xda, 0xd3, 0x8a, 0xff, 0x4f, 0xda, 0x10, 0x48, 0x7, 0xfc, 0x65, 0x58, 0x23, 0x87, 0x75, 0xb4, 0x64, 0x5d, 0x5d, 0x4f, 0x73, 0xef, 0xb7, 0x69, 0x2e, 0x46, 0x46, 0x13, 0xd, 0x11, 0xdd, 0x3d, 0xbd, 0x30, 0xd1, 0x3, 0xdf, 0x35, 0xa1, 0x22, 0x67, 0x80, 0xe3, 0x4a, 0x6a, 0xfa, 0x89, 0x2a, 0xa, 0x9c, 0xbb, 0x53, 0xb7, 0xb1, 0xbf, 0x6, 0x31, 0xf2, 0xd7, 0xcd, 0xa0, 0x96, 0x6, 0xbc, 0xf2, 0x3a, 0xb5, 0xf, 0xbf, 0x70, 0x7b, 0xba, 0xcf, 0xaa, 0xd9, 0x3c, 0x3, 0x9, 0x61, 0xd9, 0x7e, 0x93, 0xe6, 0xd8, 0xfc, 0xc, 0x2f, 0xc8, 0x10, 0xe8, 0xb, 0x8b, 0x3b, 0xd4, 0xfd, 0xfb, 0x28, 0x37, 0x25, 0x7f, 0x60, 0x7f, 0xae, 0x4, 0x78, 0x4, 0x25, 0xb7, 0x5d, 0xcb, 0xc2, 0x50, 0xb, 0x20, 0x4b, 0x95, 0x10, 0x3a, 0x1a, 0x61, 0x20, 0x14, 0x7f, 0xb5, 0x16, 0xee, 0x88, 0xe0, 0x3f, 0x89, 0x4c, 0x4f, 0x44, 0x7a, 0xd9, 0x64, 0xfd, 0x2e, 0xed, 0x13, 0xca, 0x11, 0x13, 0xac, 0x8c, 0x39, 0xbc, 0x3b, 0xe4, 0x10, 0x9e, 0x45, 0xce, 0x98, 0x84, 0xd9, 0xd3, 0xb0, 0x8d, 0xbe, 0xa4, 0xd, 0x5f, 0x76, 0x7a, 0x43, 0x5, 0xfb, 0x5c, 0x68, 0x10, 0xa2, 0x8f, 0x4f, 0x22, 0x1e, 0xa5, 0xdf, 0x4b, 0x95, 0xaa, 0x83, 0xf4, 0xd6, 0x14, 0x10, 0x9, 0xe1, 0x2, 0x56, 0x6a, 0x3e, 0x6a, 0xb9, 0x67, 0xef, 0x6c, 0xad, 0x6e, 0xa2, 0xdf, 0x90, 0x63, 0xf, 0x1f, 0x94, 0xbf, 0xa2, 0xed, 0x19, 0x32, 0x96, 0x7d, 0x92, 0x3c, 0xa2, 0x1b, 0xa0, 0xac, 0x7a, 0xb3, 0x81, 0x92, 0x4c, 0xff, 0x8c, 0xff};
uint8_t sramData[1000];
/* USER CODE END 0 */

int main(void) {
  HAL_Init();
  Nucleo_BSP_Init();

  hdma_memtomem_dma1_channel5.Instance = DMA1_Channel5;
  hdma_memtomem_dma1_channel5.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel5.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma1_channel5.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel5.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_memtomem_dma1_channel5.Init.MemDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_memtomem_dma1_channel5.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel5.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  HAL_DMA_Init(&hdma_memtomem_dma1_channel5);

  GPIOC->ODR = 0x100;
  HAL_DMA_Start(&hdma_memtomem_dma1_channel5,  (uint32_t)&flashData,  (uint32_t)&sramData, 1000);
  HAL_DMA_PollForTransfer(&hdma_memtomem_dma1_channel5, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
  GPIOC->ODR = 0x0;

  while(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin));

  hdma_memtomem_dma1_channel5.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma1_channel5.Init.MemDataAlignment = DMA_PDATAALIGN_WORD;
  HAL_DMA_Init(&hdma_memtomem_dma1_channel5);

  GPIOC->ODR = 0x100;
  HAL_DMA_Start(&hdma_memtomem_dma1_channel5,  (uint32_t)&flashData,  (uint32_t)&sramData, 250);
  HAL_DMA_PollForTransfer(&hdma_memtomem_dma1_channel5, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
  GPIOC->ODR = 0x0;

  HAL_Delay(1000);

  while(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin));

  GPIOC->ODR = 0x100;
  memcpy(sramData, flashData, 1000);
  GPIOC->ODR = 0x0;

  HAL_Delay(1000);

  while(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin));

  GPIOC->ODR = 0x100;
  for(int i = 0; i < 1000; i++)
	  sramData[i] = flashData[i];
  GPIOC->ODR = 0x0;

  /* Infinite loop */
  while (1);
}


/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
