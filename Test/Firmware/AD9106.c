#include "AD9106.h"
	
// Приватные SPI функции
__inline uint16_t AD9106_SPI_ReadWrite(uint16_t data);
__inline void AD9106_SPI_Write(uint16_t data);

__inline uint16_t AD9106_SPI_ReadWrite(uint16_t data) 
{
	uint16_t result = 0;
	while (SPI_I2S_GetFlagStatus(AD9106_SPI, SPI_I2S_FLAG_TXE) == RESET){};
	SPI_I2S_SendData(AD9106_SPI, data);
	while (SPI_I2S_GetFlagStatus(AD9106_SPI, SPI_I2S_FLAG_BSY) == SET){};
	
  SPI_BiDirectionalLineConfig(AD9106_SPI, SPI_Direction_Rx);
	while (SPI_I2S_GetFlagStatus(AD9106_SPI, SPI_I2S_FLAG_RXNE) == RESET){};
	SPI_BiDirectionalLineConfig(AD9106_SPI, SPI_Direction_Tx);	
	result = SPI_I2S_ReceiveData(AD9106_SPI);
		
	return result;
}

__inline void AD9106_SPI_Write(uint16_t data) 
{
	while (SPI_I2S_GetFlagStatus(AD9106_SPI, SPI_I2S_FLAG_TXE) == RESET){};
	SPI_I2S_SendData(AD9106_SPI, data);
	while (SPI_I2S_GetFlagStatus(AD9106_SPI, SPI_I2S_FLAG_BSY) == SET){};
}

void AD9106_Init(void)
{
	GPIO_InitTypeDef PORT;
	SPI_InitTypeDef SPI;
	
	RCC_APB2PeriphClockCmd(AD9106_SPI_PERIPH, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	// SPI pins
	PORT.GPIO_Speed = GPIO_Speed_50MHz;
	PORT.GPIO_Pin = AD9106_SPI_SCK_PIN | AD9106_SPI_MISO_PIN | AD9106_SPI_MOSI_PIN;
	PORT.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(AD9106_SPI_PORT, &PORT);
	
	// CE
	PORT.GPIO_Pin = AD9106_SPI_CE_PIN;
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(AD9106_SPI_CE_PORT, &PORT);
	
	// Reset
	PORT.GPIO_Pin = AD9106_RES_PIN;
	//PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(AD9106_RES_PORT, &PORT);
	
	// Clock Enable
	PORT.GPIO_Pin = AD9106_CLK_EN_PIN;
	GPIO_Init(AD9106_CLK_EN_PORT, &PORT);
	
	// TRIG
	PORT.GPIO_Pin = AD9106_TRG_PIN;
	//PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(AD9106_TRG_PORT, &PORT);
	
	// SPI
	SPI.SPI_Mode = SPI_Mode_Master;
	SPI.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; // на 2-4 уже глючит, НО скорей всего это из за проводного соеденения
	SPI.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI.SPI_CPOL = SPI_CPOL_Low;
	SPI.SPI_CPHA = SPI_CPHA_1Edge;
	SPI.SPI_CRCPolynomial = 7;
	SPI.SPI_DataSize = SPI_DataSize_16b;
	SPI.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(AD9106_SPI, &SPI);
	
	SPI_NSSInternalSoftwareConfig(AD9106_SPI, SPI_NSSInternalSoft_Set);
	SPI_Cmd(AD9106_SPI, ENABLE);	
	
	AD9106_TRG_LOW;
	AD9106_CE_HIGH;
	
	AD9106_RES_LOW;
	AD9106_RES_HIGH;
	
	// инициализируем работу SPI по 3 проводам! SDIO - двунаправленный. SDO - не используется.
	AD9106_WriteReg(SPICONFIG, 0x4002);
	
	AD9106_CLK_ENABLE;
}

uint16_t AD9106_ReadReg(uint16_t regAddress)
{
	uint16_t result = 0, reg = 0;
	
	AD9106_CE_LOW;
	reg = (uint16_t)(regAddress | 0x8000);
	
	result = AD9106_SPI_ReadWrite(reg);
	AD9106_CE_HIGH;
	return result;
}

void AD9106_WriteReg(uint16_t regAddress, uint16_t data)
{
	AD9106_CE_LOW;
	AD9106_SPI_Write(regAddress);
	AD9106_SPI_Write(data);
	AD9106_CE_HIGH;
}

void AD9106_RegisterDataWrite(uint8_t ADDR, uint16_t DATA)
{
	if((ADDR != 15) && (ADDR != 16)) // PAT_STATUS и RAMUPDATE пропускаем при глобальном обновлении параметров
	{
		if(	(ADDR == START_ADDR1) || (ADDR == STOP_ADDR1) ||
				(ADDR == START_ADDR2) || (ADDR == STOP_ADDR2) ||
				(ADDR == START_ADDR3) || (ADDR == STOP_ADDR3) ||
				(ADDR == START_ADDR4) || (ADDR == STOP_ADDR4))
		{
			AD9106_WriteReg(ADDR, (uint16_t)(DATA << 4)); // мега костыль, для исправления бага в адресации DDS
		}
		else
		{
			AD9106_WriteReg(ADDR, DATA);
		}
	}
}

uint16_t AD9106_RegisterDataRead(uint8_t ADDR)
{
	uint16_t value = 0xffff;

	value = AD9106_ReadReg(ADDR);
	if(value == 0xffff) // была ошибка чтения, еще одна попытка
	{
		value = AD9106_ReadReg(ADDR);
	}
	
	// мега костыль, для исправления бага в адресации DDS, возвращаем на место так сказать прочитанное значение
	if(	(ADDR == START_ADDR1) || (ADDR == STOP_ADDR1) ||
			(ADDR == START_ADDR2) || (ADDR == STOP_ADDR2) ||
			(ADDR == START_ADDR3) || (ADDR == STOP_ADDR3) ||
			(ADDR == START_ADDR4) || (ADDR == STOP_ADDR4))
	{
		value = (uint16_t)(value >> 4);
	}
	return value;
}

void AD9106_StopGeneration(void)
{
	AD9106_WriteReg(PAT_STATUS, 0x00);
	AD9106_WriteReg(RAMUPDATE, 0x01);
}

// жесткий сброс 
void AD9106_ManualReset(void)
{
	AD9106_RES_LOW;
	AD9106_RES_HIGH;
	
	// инициализируем работу SPI по 3 проводам! SDIO - двунаправленный. SDO - не используется.
	AD9106_WriteReg(SPICONFIG, 0x4002);
}

// это так же блокирует доступ к SRAM
void AD9106_UpdateSPIParams(void)
{
	AD9106_WriteReg(PAT_STATUS, 0x00); // первые 2 команды по идее избыточны...
	AD9106_WriteReg(RAMUPDATE, 0x01);
	AD9106_WriteReg(PAT_STATUS, 0x01);
	AD9106_WriteReg(RAMUPDATE, 0x01);
	
	// для запуска генерации
	AD9106_TRG_LOW;
	AD9106_TRG_HIGH;
	AD9106_TRG_LOW;
}

__inline void AD9106_TriggerIdleState(void)
{
	AD9106_TRG_HIGH;
}

__inline void AD9106_TriggerActiveState(void)
{
	AD9106_TRG_LOW;
}

bool AD9106_WritePatternToSRAM(uint16_t *dataBuf, uint16_t bufLength, uint16_t sramAddr)
{
	uint16_t i;
	if ((sramAddr < SRAMDATA) || ((sramAddr + bufLength) > SRAMDATA_END))
		return false;

	AD9106_WriteReg(PAT_STATUS, 0x4);
	AD9106_WriteReg(RAMUPDATE, 0x1);
	
	for (i = 0; i < bufLength; i++)
	{
		AD9106_WriteReg(sramAddr + i, dataBuf[i]);
	}
	
	return true;
}

bool AD9106_ReadPatternFromSRAM(uint16_t *dataBuf, uint16_t bufLength, uint16_t sramAddr)
{
	uint16_t i;
	if ((sramAddr < SRAMDATA) || ((sramAddr + bufLength) > SRAMDATA_END))
		return false;

	AD9106_WriteReg(PAT_STATUS, 0xC);
	AD9106_WriteReg(RAMUPDATE, 0x1);

	for (i = 0; i < bufLength; i++)
	{
		dataBuf[i] = AD9106_ReadReg(sramAddr + i);
	}
	
	return true;
}

void AD9106_PlayPatternFromSRAM(uint8_t nrOfDac, uint16_t startAddr, uint16_t stopAddr, uint16_t nrOfWaveCycles, uint16_t startDelay)
{
	AD9106_WriteReg(WAV2_1CONFIG, 0x30);
	AD9106_WriteReg(DDS_TW1, 0x1000); //frequency settings i dont know if its working with arbitrary pattern
	//AD9106_WriteReg(DAC1_CST, 0xA200);
	AD9106_WriteReg(DAC1_DGAIN, 0x4000); //amplitude gain
	//AD9106_WriteReg(DAC1RSET, 0x8002);
	AD9106_WriteReg(DACxRANGE, 0x0000);
	AD9106_WriteReg(PAT_TIMEBASE, 0x0FF1); //this register changes the time between samples
	AD9106_WriteReg(PAT_PERIOD, 0x8000);
	
	//AD9106_WriteReg(PAT_TYPE, 0x0001); //pattern repeats DAC4_3PATx number of times
	//AD9106_WriteReg(DAC2_1PATx, 0x0202); //pattern for DAC4 repeats 2 times
	//AD9106_WriteReg(PAT_STATUS, 0);
	
	AD9106_WriteReg(START_ADDR1, startAddr << 4); //read register description!!! this value starts from 0 and is shifted left 4 places, the same with STOP_ADDR4
	AD9106_WriteReg(STOP_ADDR1, stopAddr << 4);
	AD9106_WriteReg(START_DLY1, 0x00);
	
	AD9106_WriteReg(PAT_STATUS, 0x01);
	AD9106_WriteReg(RAMUPDATE, 0x01);
}
//////////////--------------------------------------------------к УДАЛЕНИЮ !------------------------------------------------------------------//////////////

void AD9106_Test()
{
	static uint16_t ddd[1024];//, ddd_r[1024];
	uint16_t i;
	int16_t data;
	for (i = 0; i < 1024; i++)
	{
		data = (int16_t)(2048 - i * 4);
		
		ddd[i]=(uint16_t)(data << 3);//(i*2) << 4;
	}
	//dd[0] =(uint16_t)(0xfff << 3); 
	AD9106_WritePatternToSRAM(ddd, 1024, SRAMDATA);
	//AD9106_ReadPatternFromSRAM(ddd_r, 1023, SRAMDATA);
	
	AD9106_PlayPatternFromSRAM(0, 0, 1023, 0, 0);
	
	AD9106_TRG_LOW;
	AD9106_TRG_HIGH;
	AD9106_TRG_LOW;
	
}

/*
void AD9106_LocalDataWrite(uint8_t reg_num, uint16_t reg_data)
{
	if(reg_num <= AD9106_REG_NUM_MAX)
	{
		if((reg_num != 15) && (reg_num != 16)) // PAT_STATUS и RAMUPDATE пропускаем при глобальном обновлении параметров
		{
			AD9106_RegistersLocalData[reg_num] = reg_data;
		}
	}
}

uint16_t AD9106_LocalDataRead(uint8_t reg_num)
{
	if(reg_num <= AD9106_REG_NUM_MAX)
	{
		return AD9106_RegistersLocalData[reg_num];
	}
	return 0xffff;
}

void AD9106_LocalRegDataUpdate(void)
{
	uint8_t i;
	for(i=0;i<=AD9106_REG_NUM_MAX;i++)
	{
		AD9106_LocalRegDataRead(i);
	}
}
*/
