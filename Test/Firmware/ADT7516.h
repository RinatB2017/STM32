#ifndef _ADT7516_H_
#define _ADT7516_H_

#include "ADT7516_R.h"
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_spi.h>
#include "cmsis_os.h"

// Используемый по умолчанию порт SPI
#ifndef ADT7516_SPI
#define ADT7516_SPI										SPI2
#define ADT7516_SPI_PORT							GPIOB
#define ADT7516_SPI_PERIPH						RCC_APB1Periph_SPI2
#define ADT7516_SPI_SCK_PIN  					GPIO_Pin_13
#define ADT7516_SPI_MISO_PIN 					GPIO_Pin_14
#define ADT7516_SPI_MOSI_PIN 					GPIO_Pin_15
#endif

// Chip enable
#ifndef ADT7516_SPI_CE_PIN
#define ADT7516_SPI_CE_PORT						GPIOB
#define ADT7516_SPI_CE_PIN						GPIO_Pin_12
#endif

// Chip enable configuration
#define ADT7516_CE_LOW								GPIO_ResetBits(ADT7516_SPI_CE_PORT, ADT7516_SPI_CE_PIN)
#define ADT7516_CE_HIGH								GPIO_SetBits(ADT7516_SPI_CE_PORT, ADT7516_SPI_CE_PIN)

#define ADT7516_SPI_OPERATION_READ 		0x91
#define ADT7516_SPI_OPERATION_WRITE 	0x90
#define ADT7516_SPI_MANUAL_RESET		 	0x80

// b0 = 1 Start monitoring
// b1-2 = 00 AIN1 and AIN2 selected
// b3 = 1 AIN3 selected
// b5 = 1 Disable INT/INT output
// b6 = 0 INT/INT output polarity, active low
// b7 = 0 Power-Down OFF
#define ADT7516_SPI_INIT_CNTRL_CONF1 	0x29

// b0-2 = 000 For single mode only...
// b4 = 0 Round robin conversion cycle
// b5 = 0 Enable averaging
// b6 = 0 Disable SMBus timeout
// b7 = 0 Software Reset
#define ADT7516_SPI_INIT_CNTRL_CONF2 	0x00

// b0 = 0 ADC clock at 1.4 kHz
// b1 = 1 10-bit resolution
// b3 = 1 DAC configuration register and LDAC configuration register control updating of DAC outputs
// b4 = 0 ADC reference - internal VREF
// b5 = 0 DAC A - normal mode
// b6 = 0 DAC B - normal mode
#define ADT7516_SPI_INIT_CNTRL_CONF3 	0x0A

uint8_t ADT7516_ReadReg(uint8_t regAddress);
void ADT7516_WriteReg(uint8_t regAddress, uint8_t data);

void ADT7516_Init(void);
void ADT7516_ManualReset(void);

uint16_t ADT7516_ReadADC(uint8_t channel);
float ADT7516_ReadInternalTemp(void);
void ADT7516_WriteDAC(uint8_t channel, uint16_t data);

#endif // _ADT7516_H_
