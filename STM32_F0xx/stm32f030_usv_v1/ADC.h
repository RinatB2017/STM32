
#include <stm32f0xx_conf.h>

void ADC_init()
{
	RCC->CR2 |= RCC_CR2_HSI14ON;//задать тактирование от HSI14

	while (!(RCC->CR2 & RCC_CR2_HSI14RDY));//задать тактирование от HSI14

	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; //RCC_APB2ENR_ADCEN//включить такты на управление регистрами ADC

	ADC1->CHSELR |= ADC_CHSELR_CHSEL16;     //Канал датчика температуры
//	ADC1->CHSELR |= ADC_CHSELR_CHSEL1;
	ADC1->CR |= ADC_CR_ADCAL;              // Калибровка датчика температуры
//	ADC1->CR |=  ADC_CR_ADEN;

	while ((ADC1->CR & ADC_CR_ADCAL) != 0);

	ADC1->SMPR |= ADC_SMPR1_SMPR_0 | ADC_SMPR1_SMPR_1 | ADC_SMPR1_SMPR_2;

	ADC1->CFGR1 &= (uint32_t)(~ADC_CFGR1_CONT);     //Выбор триггера
//	ADC1->CFGR1 |= ADC_CFGR1_EXTSEL_0 | ADC_CFGR1_EXTSEL_1;     //Выбор триггера TIM3

	ADC1->CFGR1 |= ADC_CFGR1_EXTEN_0;
	ADC1->CFGR1 |= ADC_CFGR1_EXTSEL_0 | ADC_CFGR1_EXTSEL_1;

	ADC->CCR |= ADC_CCR_TSEN;

	ADC1->CR |= ADC_CR_ADEN;
	while ((ADC1->ISR & ADC_ISR_ADRDY) == 0);

	ADC1->CR |= ADC_CR_ADSTART;	// start adc
}
	
unsigned char ADC_get_val_ch0(void)
{
	unsigned int temp_adc=0;
	ADC_ChannelConfig(ADC1, ADC_Channel_0, ADC_SampleTime_239_5Cycles);
	ADC_StartOfConversion(ADC);
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)){}
	temp_adc=ADC_GetConversionValue(ADC);
	//clear EOC flag
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
}
