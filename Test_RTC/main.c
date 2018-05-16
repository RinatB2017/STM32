#include "stm32f10x.h"
#include "stdio.h"

#include "unix_time.h"

#include "usart.h"

uint32_t timer=1384850400+14400;
unix_cal  unix_time;
unix_cal  old_unix_time;
char buf[1000];

void  RTC_INIT  (void)                                              //Инициализация RTC
{
	if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)             //Проверка работы часов, если не включены, то инициализировать
	{
		RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;		//Включить тактирование PWR и Backup
		PWR->CR |= PWR_CR_DBP;                                      //Разрешить доступ к Backup области
		RCC->BDCR |= RCC_BDCR_BDRST;                                //Сбросить Backup область
		RCC->BDCR &= ~RCC_BDCR_BDRST;
		RCC->BDCR |= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSE;        	//Выбрать LSE источник (кварц 32768) и подать тактирование
		RCC->BDCR |= RCC_BDCR_LSEON;                                //Включить LSE
		while ((RCC->BDCR & RCC_BDCR_LSEON) != RCC_BDCR_LSEON){} 	//Дождаться включения
		BKP->RTCCR |= 3;                                            //калибровка RTC
		while (!(RTC->CRL & RTC_CRL_RTOFF));                        //проверить закончены ли изменения регистров RTC
		RTC->CRL  |=  RTC_CRL_CNF;                                  //Разрешить Запись в регистры RTC
		RTC->PRLL  = 0x7FFF;                                        //Настроит делитель на 32768 (32767+1)
		RTC->CRL  &=  ~RTC_CRL_CNF;                                 //Запретить запись в регистры RTC
		while (!(RTC->CRL & RTC_CRL_RTOFF));                        //Дождаться окончания записи
		RTC->CRL &= (uint16_t)~RTC_CRL_RSF;                         //Синхронизировать RTC
		while((RTC->CRL & RTC_CRL_RSF) != RTC_CRL_RSF){}            //Дождаться синхронизации
		PWR->CR &= ~PWR_CR_DBP;                                     //запретить доступ к Backup области
	}
}

uint32_t RTC_GetCounter_(void)                                      //Получить значение счетчика
{
	return  (uint32_t)((RTC->CNTH << 16) | RTC->CNTL);
}

void RTC_SetCounter_(uint32_t count)                                //Записать новое значение счетчика
{
	RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;  		//включить тактирование PWR и Backup
	PWR->CR |= PWR_CR_DBP;                                          //разрешить доступ к Backup области
	while (!(RTC->CRL & RTC_CRL_RTOFF));                            //проверить закончены ли изменения регистров RTC
	RTC->CRL |= RTC_CRL_CNF;                                        //Разрешить Запись в регистры RTC
	RTC->CNTH = count>>16;                                          //записать новое значение счетного регистра
	RTC->CNTL = count;
	RTC->CRL &= ~RTC_CRL_CNF;                                       //Запретить запись в регистры RTC
	while (!(RTC->CRL & RTC_CRL_RTOFF));                            //Дождаться окончания записи
	PWR->CR &= ~PWR_CR_DBP;                                         //запретить доступ к Backup области
}

char check_new_time(void)
{
	if(unix_time.hour != old_unix_time.hour)	return 1;
	if(unix_time.min  != old_unix_time.min)		return 1;
	if(unix_time.sec  != old_unix_time.sec)		return 1;
	return 0;
}

int main(void)
{
	usart_init();

	RTC_INIT();                                                     //Инициализируем RTC
	RTC_SetCounter_(timer);       									//Записать новое значение счетчика

	old_unix_time.hour = 0;
	old_unix_time.min = 0;
	old_unix_time.sec = 0;
	while(1)
	{
		 timer=RTC_GetCounter_();
		 timer_to_cal (timer, &unix_time);

		 if(check_new_time())
		 {
			 sprintf(buf, "%d.%d.%d\n", unix_time.hour, unix_time.min, unix_time.sec);
			 usart_put_str(buf);

			 old_unix_time.hour = unix_time.hour;
			 old_unix_time.min  = unix_time.min;
			 old_unix_time.sec  = unix_time.sec;
		 }
	}
}
