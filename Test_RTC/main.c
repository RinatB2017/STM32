#include "stm32f10x.h"
#include "stdio.h"

#include "unix_time.h"

#include "usart.h"

uint32_t timer=1384850400+14400;
unix_cal  unix_time;
unix_cal  old_unix_time;
char buf[1000];

void  RTC_INIT  (void)                                              //������������� RTC
{
	if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)             //�������� ������ �����, ���� �� ��������, �� ����������������
	{
		RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;		//�������� ������������ PWR � Backup
		PWR->CR |= PWR_CR_DBP;                                      //��������� ������ � Backup �������
		RCC->BDCR |= RCC_BDCR_BDRST;                                //�������� Backup �������
		RCC->BDCR &= ~RCC_BDCR_BDRST;
		RCC->BDCR |= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSE;        	//������� LSE �������� (����� 32768) � ������ ������������
		RCC->BDCR |= RCC_BDCR_LSEON;                                //�������� LSE
		while ((RCC->BDCR & RCC_BDCR_LSEON) != RCC_BDCR_LSEON){} 	//��������� ���������
		BKP->RTCCR |= 3;                                            //���������� RTC
		while (!(RTC->CRL & RTC_CRL_RTOFF));                        //��������� ��������� �� ��������� ��������� RTC
		RTC->CRL  |=  RTC_CRL_CNF;                                  //��������� ������ � �������� RTC
		RTC->PRLL  = 0x7FFF;                                        //�������� �������� �� 32768 (32767+1)
		RTC->CRL  &=  ~RTC_CRL_CNF;                                 //��������� ������ � �������� RTC
		while (!(RTC->CRL & RTC_CRL_RTOFF));                        //��������� ��������� ������
		RTC->CRL &= (uint16_t)~RTC_CRL_RSF;                         //���������������� RTC
		while((RTC->CRL & RTC_CRL_RSF) != RTC_CRL_RSF){}            //��������� �������������
		PWR->CR &= ~PWR_CR_DBP;                                     //��������� ������ � Backup �������
	}
}

uint32_t RTC_GetCounter_(void)                                      //�������� �������� ��������
{
	return  (uint32_t)((RTC->CNTH << 16) | RTC->CNTL);
}

void RTC_SetCounter_(uint32_t count)                                //�������� ����� �������� ��������
{
	RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;  		//�������� ������������ PWR � Backup
	PWR->CR |= PWR_CR_DBP;                                          //��������� ������ � Backup �������
	while (!(RTC->CRL & RTC_CRL_RTOFF));                            //��������� ��������� �� ��������� ��������� RTC
	RTC->CRL |= RTC_CRL_CNF;                                        //��������� ������ � �������� RTC
	RTC->CNTH = count>>16;                                          //�������� ����� �������� �������� ��������
	RTC->CNTL = count;
	RTC->CRL &= ~RTC_CRL_CNF;                                       //��������� ������ � �������� RTC
	while (!(RTC->CRL & RTC_CRL_RTOFF));                            //��������� ��������� ������
	PWR->CR &= ~PWR_CR_DBP;                                         //��������� ������ � Backup �������
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

	RTC_INIT();                                                     //�������������� RTC
	RTC_SetCounter_(timer);       									//�������� ����� �������� ��������

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
