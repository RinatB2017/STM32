#ifndef _AD9106_H_
#define _AD9106_H_

#include "AD9106_R.h"
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_spi.h>
#include <stdbool.h>

// Используемый по умолчанию порт SPI
#ifndef AD9106_SPI
#define AD9106_SPI						SPI1
#define AD9106_SPI_PERIPH			RCC_APB2Periph_SPI1
#define AD9106_SPI_PORT				GPIOA
#define AD9106_SPI_SCK_PIN  	GPIO_Pin_5
#define AD9106_SPI_MISO_PIN 	GPIO_Pin_6
#define AD9106_SPI_MOSI_PIN 	GPIO_Pin_7
#endif

// CS \ CE
#ifndef AD9106_SPI_CE_PIN
#define AD9106_SPI_CE_PORT		GPIOA
#define AD9106_SPI_CE_PIN			GPIO_Pin_4
#endif

// Reset
#ifndef AD9106_RES_PIN
#define AD9106_RES_PORT				GPIOA
#define AD9106_RES_PIN				GPIO_Pin_3
#endif

// TRG
#ifndef AD9106_TRG_PIN
#define AD9106_TRG_PORT				GPIOA
#define AD9106_TRG_PIN				GPIO_Pin_8 
#endif

// Clock 125MHz enable
#ifndef AD9106_CLK_EN_PIN
#define AD9106_CLK_EN_PORT		GPIOA
#define AD9106_CLK_EN_PIN			GPIO_Pin_2
#endif

#define AD9106_CE_LOW					GPIO_ResetBits(AD9106_SPI_CE_PORT, AD9106_SPI_CE_PIN)
#define AD9106_CE_HIGH				GPIO_SetBits(AD9106_SPI_CE_PORT, AD9106_SPI_CE_PIN)

#define AD9106_TRG_LOW				GPIO_ResetBits(AD9106_TRG_PORT, AD9106_TRG_PIN)
#define AD9106_TRG_HIGH				GPIO_SetBits(AD9106_TRG_PORT, AD9106_TRG_PIN)

#define AD9106_RES_LOW				GPIO_ResetBits(AD9106_RES_PORT, AD9106_RES_PIN)
#define AD9106_RES_HIGH				GPIO_SetBits(AD9106_RES_PORT, AD9106_RES_PIN)

// Clock 125MHz configuration
#define AD9106_CLK_ENABLE			GPIO_SetBits(AD9106_CLK_EN_PORT, AD9106_CLK_EN_PIN)
#define AD9106_CLK_DISABLE		GPIO_ResetBits(AD9106_CLK_EN_PORT, AD9106_CLK_EN_PIN)

void AD9106_Init(void);

// Эти функции для прямой работы с SPI
uint16_t AD9106_ReadReg(uint16_t regAddress);
void AD9106_WriteReg(uint16_t regAddress, uint16_t data);

void AD9106_UpdateSPIParams(void); // Обновить параметры в соотв.с последними записаными
void AD9106_ManualReset(void); // полный сброс до настроек по умолчанию
void AD9106_StopGeneration(void); // остановить генерацию сигнала

void AD9106_RegisterDataWrite(uint8_t ADDR, uint16_t DATA);
uint16_t AD9106_RegisterDataRead(uint8_t ADDR);

// Работа с SRAM. запись с адреса 0х6000 !!!
bool AD9106_WritePatternToSRAM(uint16_t *dataBuf, uint16_t bufLength, uint16_t sramAddr);
bool AD9106_ReadPatternFromSRAM(uint16_t *dataBuf, uint16_t bufLength, uint16_t sramAddr);

// Проигрывание паттерна. адресация с 0х0 по 0хFFF !!!!!! НЕ с 0х6000 !!!
void AD9106_PlayPatternFromSRAM(uint8_t nrOfDac, uint16_t startAddr, uint16_t stopAddr, uint16_t nrOfWaveCycles, uint16_t startDelay);
void AD9106_PatternPeriod(uint16_t PeriodBase, uint16_t Period);

// Настройки по каналам...
void AD9106_ChDigitalOffset(uint8_t Channel, uint16_t Offset);
void AD9106_ChPhaseOffset(uint8_t Channel, uint16_t PhaseOffset);
void AD9106_ChConstant(uint8_t Channel, uint16_t Constant);

__inline void AD9106_TriggerIdleState(void);
__inline void AD9106_TriggerActiveState(void);

// потом удалить.....
void AD9106_Test(void);

#endif // _AD9106_H_
