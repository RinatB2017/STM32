#include "ADT7516.h"

// Приватные SPI функции
__inline uint8_t ADT7516_SPI_ReadWrite(uint8_t data);
__inline void ADT7516_SPI_Write(uint8_t data);

__inline uint8_t ADT7516_SPI_ReadWrite(uint8_t data) 
{
	uint16_t result = 0;
	while (SPI_I2S_GetFlagStatus(ADT7516_SPI, SPI_I2S_FLAG_TXE) == RESET){};
	SPI_I2S_SendData(ADT7516_SPI, data);

	while (SPI_I2S_GetFlagStatus(ADT7516_SPI, SPI_I2S_FLAG_RXNE) == RESET){};
	result = SPI_I2S_ReceiveData(ADT7516_SPI);
		
	return result;
}

__inline void ADT7516_SPI_Write(uint8_t data) 
{
	while (SPI_I2S_GetFlagStatus(ADT7516_SPI, SPI_I2S_FLAG_TXE) == RESET){};
	SPI_I2S_SendData(ADT7516_SPI, data);
		
	while (SPI_I2S_GetFlagStatus(ADT7516_SPI, SPI_I2S_FLAG_RXNE) == RESET){};
	SPI_I2S_ReceiveData(ADT7516_SPI);
}

void ADT7516_Init(void)
{
	GPIO_InitTypeDef PORT;
	SPI_InitTypeDef SPI;
	
	RCC_APB1PeriphClockCmd(ADT7516_SPI_PERIPH, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	// SPI pins
	PORT.GPIO_Speed = GPIO_Speed_50MHz;
	PORT.GPIO_Pin = ADT7516_SPI_SCK_PIN | ADT7516_SPI_MOSI_PIN;
	PORT.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(ADT7516_SPI_PORT, &PORT);
	
	PORT.GPIO_Speed = GPIO_Speed_50MHz;
	PORT.GPIO_Pin = ADT7516_SPI_MISO_PIN;
	PORT.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(ADT7516_SPI_PORT, &PORT);
	
	// CE
	PORT.GPIO_Pin = ADT7516_SPI_CE_PIN;
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(ADT7516_SPI_CE_PORT, &PORT);
	
	// SPI
	SPI.SPI_Mode = SPI_Mode_Master;
	SPI.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI.SPI_CPOL = SPI_CPOL_Low;
	SPI.SPI_CPHA = SPI_CPHA_1Edge;
	SPI.SPI_CRCPolynomial = 7;
	SPI.SPI_DataSize = SPI_DataSize_8b;
	SPI.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(ADT7516_SPI, &SPI);
	
	SPI_NSSInternalSoftwareConfig(ADT7516_SPI, SPI_NSSInternalSoft_Set);
	SPI_Cmd(ADT7516_SPI, ENABLE);	
	
	// spi select
	ADT7516_CE_HIGH;
	osDelay(1);
	ADT7516_CE_LOW;
	osDelay(1);
	ADT7516_CE_HIGH;
	osDelay(1);
	ADT7516_CE_LOW;
	osDelay(1);
	ADT7516_CE_HIGH;
	osDelay(1);
	ADT7516_CE_LOW;
	osDelay(1);
	ADT7516_CE_HIGH;

	ADT7516_WriteReg(CNTRL_CONF1, ADT7516_SPI_INIT_CNTRL_CONF1);
	ADT7516_WriteReg(CNTRL_CONF2, ADT7516_SPI_INIT_CNTRL_CONF2);
	ADT7516_WriteReg(CNTRL_CONF3, ADT7516_SPI_INIT_CNTRL_CONF3);
}

uint8_t ADT7516_ReadReg(uint8_t regAddress)
{
	uint8_t result = 0;
	
	ADT7516_CE_LOW;
	ADT7516_SPI_Write(ADT7516_SPI_OPERATION_WRITE);
	ADT7516_SPI_Write(regAddress);
	ADT7516_CE_HIGH;
	
	ADT7516_CE_LOW;
	ADT7516_SPI_Write(ADT7516_SPI_OPERATION_READ);
	result = ADT7516_SPI_ReadWrite(0xff);
	ADT7516_CE_HIGH;
	
	return result;
}

void ADT7516_WriteReg(uint8_t regAddress, uint8_t data)
{
	ADT7516_CE_LOW;
	ADT7516_SPI_Write(ADT7516_SPI_OPERATION_WRITE);
	ADT7516_SPI_Write(regAddress);
	ADT7516_SPI_Write(data);
	ADT7516_CE_HIGH;
}

void ADT7516_ManualReset(void)
{
	ADT7516_WriteReg(CNTRL_CONF2, ADT7516_SPI_MANUAL_RESET);
	
	// spi select
	ADT7516_CE_HIGH;
	osDelay(1);
	ADT7516_CE_LOW;
	osDelay(1);
	ADT7516_CE_HIGH;
	osDelay(1);
	ADT7516_CE_LOW;
	osDelay(1);
	ADT7516_CE_HIGH;
	osDelay(1);
	ADT7516_CE_LOW;
	osDelay(1);
	ADT7516_CE_HIGH;
	
	ADT7516_WriteReg(CNTRL_CONF1, ADT7516_SPI_INIT_CNTRL_CONF1);
	ADT7516_WriteReg(CNTRL_CONF2, ADT7516_SPI_INIT_CNTRL_CONF2);
	ADT7516_WriteReg(CNTRL_CONF3, ADT7516_SPI_INIT_CNTRL_CONF3);
}

uint16_t ADT7516_ReadADC(uint8_t channel)
{
	uint16_t value = 0;
	
	switch(channel)
	{
		case 0:
			value = (uint16_t)(ADT7516_ReadReg(EXT_TMP_AIN14_L) & 0x3);
			value = (uint16_t)(value + (ADT7516_ReadReg(EXT_TMP_AIN1_M) << 2));
		break;
		
		case 1:
			value = (uint16_t)((ADT7516_ReadReg(EXT_TMP_AIN14_L) >> 2) & 0x3);
			value = (uint16_t)(value + (ADT7516_ReadReg(AIN2_M) << 2));			
		break;
				
		case 2:
			value = (uint16_t)((ADT7516_ReadReg(EXT_TMP_AIN14_L) >> 4) & 0x3);
			value = (uint16_t)(value + (ADT7516_ReadReg(AIN3_M) << 2));			
		break;
						
		case 3:
			value = (uint16_t)((ADT7516_ReadReg(EXT_TMP_AIN14_L) >> 6) & 0x3);
			value = (uint16_t)(value + (ADT7516_ReadReg(AIN4_M) << 2));			
		break;
								
		default:
		break;
	}
	
	return value;
}

float ADT7516_ReadInternalTemp(void)
{
	int16_t value = 0;
	float temp = 0.0;
	
	// читаем первые 2 младших бита
	value = (int16_t)(ADT7516_ReadReg(INT_TMP_VDD_L) & 0x3);
	
	// читаем оставшиеся 8 бит
	value = (int16_t)(value + (ADT7516_ReadReg(INT_TMP_M) << 2));
	
	if(value > -1)
	{
		temp = (float)((float)value / 4.0);
	}
	else
	{
		temp = (float)((float)(value - 512)/ 4.0);
	}		
	
	return temp;
}

void ADT7516_WriteDAC(uint8_t channel, uint16_t data)
{
	switch(channel)
	{
		case 0:
			ADT7516_WriteReg(DAC_A_L, (uint8_t)((data & 0xF) << 4));
			ADT7516_WriteReg(DAC_A_M, (uint8_t)(data >> 4));
			ADT7516_WriteReg(LDAC_CONF, 0x31);	
		break;
		
		case 1:
			ADT7516_WriteReg(DAC_B_L, (uint8_t)((data & 0xF) << 4));
			ADT7516_WriteReg(DAC_B_M, (uint8_t)(data >> 4));	
			ADT7516_WriteReg(LDAC_CONF, 0x32);			
		break;
				
		case 2:
			ADT7516_WriteReg(DAC_C_L, (uint8_t)((data & 0xF) << 4));
			ADT7516_WriteReg(DAC_C_M, (uint8_t)(data >> 4));		
			ADT7516_WriteReg(LDAC_CONF, 0x34);				
		break;
						
		case 3:
			ADT7516_WriteReg(DAC_D_L, (uint8_t)((data & 0xF) << 4));
			ADT7516_WriteReg(DAC_D_M, (uint8_t)(data >> 4));
			ADT7516_WriteReg(LDAC_CONF, 0x38);			
		break;
								
		default:	
		break;
	}
}
