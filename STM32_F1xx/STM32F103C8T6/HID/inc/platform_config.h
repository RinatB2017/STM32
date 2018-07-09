/**
 ******************************************************************************
 * @file    platform_config.h
 * @author  MCD Application Team
 * @version V4.0.0
 * @date    21-January-2013
 * @brief   Evaluation board specific configuration file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

#include <stm32f10x_gpio.h>
//TODO
#define RCC_GPIO RCC_APB2Periph_GPIOB
//---


/* Includes ------------------------------------------------------------------*/
#if defined (STM32F10X_MD) || defined (STM32F10X_HD) || defined (STM32F10X_XL)
#include "stm32f10x.h"
#else
#error "Missing define: USE_STM3210B_EVAL, USE_STM3210E_EVAL"

#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line corresponding to the STMicroelectronics evaluation board
   used to run the example */
#define USE_STM3210B_EVAL
//#define USE_STM3210E_EVAL
//#define USE_STM32L152_EVAL
//#define USE_STM32L152D_EVAL
//#define USE_STM32373C_EVAL
// #define USE_STM32303C_EVAL
#endif

/*Unique Devices IDs register set*/

#define         ID1          (0x1FFFF7E8)
#define         ID2          (0x1FFFF7EC)
#define         ID3          (0x1FFFF7F0)

/* Define the STM32F10x hardware depending on the used evaluation board */

#define BTN1_PORT GPIOC
#define BTN1_PIN  GPIO_Pin_0
#define BTN2_PORT GPIOD
#define BTN2_PIN  GPIO_Pin_12

#define LED_PORT  GPIOB

#define LED1_PIN  GPIO_Pin_6
#define LED2_PIN  GPIO_Pin_5
#define LED3_PIN  GPIO_Pin_7

#define USB_DISCONNECT                      GPIOC
#define USB_DISCONNECT_PIN                  GPIO_Pin_13

#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOD

#define RCC_APB2Periph_ALLGPIO              (RCC_APB2Periph_GPIOA \
		| RCC_APB2Periph_GPIOB \
		| RCC_APB2Periph_GPIOC \
		| RCC_APB2Periph_GPIOD \
		| RCC_APB2Periph_GPIOE )

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

