#include <pelco.h>

#define APBCLK 48000000UL // ТАКТОВАЯ ЧАСТОТА

#define  on_canal_2 	GPIOA->ODR |= GPIO_BRR_BR_0		//
#define  off_canal_2 	GPIOA->ODR &= ~GPIO_BRR_BR_0	//
#define  setout_485 	GPIOA->ODR |= GPIO_BRR_BR_4		//
#define  setin_485 	 	GPIOA->ODR &= ~GPIO_BRR_BR_4	//

#define  time_rotate 	5		// ВРЕМЯ ДЛЯ ПОВОРОТА КАМЕРЫ ПЕРЕД ВКЛЮЧЕНИЕМ ДВОРНИКА
#define  pause_mul 	 	86400	// секунд в сутках

char	shadow =0;	// ПЕРЕМЕННАЯ СЛЕЖЕНИЕ ЗА СОСТОЯНИЕМ
char	myaddr;		// ПЕРЕМЕННАЯ АДРЕСА МОДУЛЯ
char	myaddr_tmp;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ  АДРЕСА МОДУЛЯ

char	addr_canal_2;		// ПЕРЕМЕННАЯ АДРЕСА КАМЕРЫ
short	pulse_time_canal_2;	// ПЕРЕМЕННАЯ ВРЕМЯ ПОМЫВКИ
long	pause_time_canal_2;	// ПЕРЕМЕННАЯ ВРЕМЯ МЕЖДУ ПОМЫВКАМИ
char	work_preset_canal_2;// ПЕРЕМЕННАЯ ДЛЯ СОХРАНЕНИЯ ПОЛОЖЕНИЯ КАМЕРЫ ПЕРЕД ПОМЫВКОЙ
char	tmp_preset_canal_2;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ  ДЛЯ СОХРАНЕНИЯ ПОЛОЖЕНИЯ КАМЕРЫ ПЕРЕД ПОМЫВКОЙ

char	want_canal_2 = 0;		// ПЕРЕМЕННАЯ УКАЗЫВАЮЩАЯ НА НЕОБХОДИМОСТЬ ПОМЫВКИ
char	tmp_want_canal_2 = 0;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ

short	tmp_pulse_time_canal_2 = 0;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ
long	tmp_pause_time_canal_2 = 2;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ

short	tmp_rotate_time_canal_2 = 0;// ВРЕМЕННАЯ ПЕРЕМЕННАЯ
short	tmp_wiper_time_canal_2 = 0;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ
short	tmp_pomp_time_canal_2 = 0;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ

long	time_rain = 0;		// ПЕРЕМЕННАЯ ПЕРИОД ДВОРНИКА ВО ВРЕМЯ ДОЖДЯ
short	tmp_time_rain = 2;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ

char	wiper = 0;		// ПЕРЕМЕННАЯ ДВОРНИК
char	rain = 0;		// ПЕРЕМЕННАЯ ДОЖДЬ
char	water = 0;		// ПЕРЕМЕННАЯ ВОДА
char	manual = 0;		// ПЕРЕМЕННАЯ ПРИНУДИТЕЛЬНАЯ ПОМЫВКА
char	wiper_tmp = 0;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ
char	rain_tmp = 0;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ
char	water_tmp = 0;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ
char	manual_tmp = 0;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ

long	mif_time_tmp = 0x400;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ
long	mif_time = 0x4F5;		// 8MhZ == 500 mSec//  ДИСКРЕТНОСТЬ

char	d2=0;	// ВРЕМЕННАЯ ПЕРЕМЕННАЯ

void init_gpio_mif(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;     //Включаем тактирование GPIOA
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;     //Включаем тактирование GPIOB

	//=============================================  A0 ==============================================================
	GPIOA->MODER |= IO_0_OUT;				// Конфигурация порта PA0  out
	//=============================================  A1 ==============================================================
	GPIOA->MODER |= IO_1_IN; 				// Конфигурация порта PA1 wather
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR1_0;	// подтяжка up
	//=============================================  A2 ==============================================================
	GPIOA->MODER |= IO_2_OUT;				// Конфигурация порта PA2
	//=============================================  A3 ==============================================================
	GPIOA->MODER |= IO_3_IN; 				// Конфигурация порта PA3
	//=============================================  A4 ==============================================================
	GPIOA->MODER |= IO_4_OUT;				// Конфигурация порта PA4 ----------------485
	GPIOA->MODER |= IO_5_IN; 				// Конфигурация порта PA5
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR5_0;	// подтяжка up
	GPIOA->MODER |= IO_6_IN; 				// Конфигурация порта PA6
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR6_0;	// подтяжка up
	//=============================================  A7 ==============================================================
	GPIOA->MODER |= IO_7_IN;				// Конфигурация порта PA7 addr0
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR7_0;	// подтяжка up
	//=============================================  A9 ==============================================================addr2
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR9_0;	// подтяжка up
	//=============================================  A10 =============================================================addr1
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0;	// подтяжка up
	//=============================================  B1 ==============================================================
	GPIOB->MODER |=	IO_1_OUT; 				// Конфиурация порта PB1
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR13_0;	// подтяжка up
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR14_0;	// подтяжка up
}
//================================================================================================================
void  init_EXTI_mif()
{
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Включили тактирование модуля SYSCFG

	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PA; // Конфигурируем вывод РА3(макрос берём из файла stm32f0xx.h)-------PA3
	//		SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PA; // Конфигурируем вывод РА1(макрос берём из файла stm32f0xx.h)-------PA1
	//		SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PA; // Конфигурируем вывод РА5(макрос берём из файла stm32f0xx.h)
	EXTI->IMR = 1<<3; // Разрешил прерывание//		EXTI->IMR = 0x0002; // Разрешил прерывание------------PA3
	//		EXTI->IMR = 1<<1; // Разрешил прерывание//		EXTI->IMR = 0x0002; // Разрешил прерывание------------PA1
	//		EXTI->IMR = 1<<5; // Разрешил прерывание
	EXTI->FTSR = 1<<3; // Разрешил срабатывание по спаду-----PA3
	//		EXTI->FTSR = 1<<1; // Разрешил срабатывание по спаду-----PA1
	//		EXTI->RTSR = 0x0002; // Разрешил срабатывание по фронту
	//		EXTI->RTSR = 1<<5; // Разрешил срабатывание по фронту
	// Конфигурируем NVIC для внешних прерываний
	NVIC_EnableIRQ(EXTI2_3_IRQn); // Разрешаем прерывание на линии EXTI2_3----------PA3
	//		NVIC_EnableIRQ(EXTI0_1_IRQn); // Разрешаем прерывание на линии EXTI0_1----------PA1
	//		NVIC_EnableIRQ(EXTI4_15_IRQn); // Разрешаем прерывание на линии a5
}
//================================================================================================================
void mif_loud_var_from_flash(void)
{
	addr_canal_2 = read_var(FLASH_PAGE_CAN2_ADDR_T,0);//ПЕРЕМЕННАЯ АДРЕС КАМЕРЫ
	pulse_time_canal_2 = read_var(FLASH_PAGE_CAN2_Timers,0);//ПЕРЕМЕННАЯ ВРЕМЯ ПОМЫВКИ
	pause_time_canal_2 = read_var(FLASH_PAGE_CAN2_Timers,1);////ПЕРЕМЕННАЯ ВРЕМЯ МЕЖДУ ПОМЫВКАМИ
	work_preset_canal_2 = read_var(FLASH_PAGE_CAN2_Preset,0);//ПЕРЕМЕННАЯ  ДЛЯ СОХРАНЕНИЯ ПОЛОЖЕНИЯ ПРЕСЕТА КАМЕРЫ НА ПОМЫВКУ
	tmp_preset_canal_2 = read_var(FLASH_PAGE_CAN2_Preset,1);//ПЕРЕМЕННАЯ  ДЛЯ СОХРАНЕНИЯ ПОЛОЖЕНИЯ ПРЕСЕТА КАМЕРЫ ПЕРЕД ПОМЫВКОЙ
	time_rain = read_var(FLASH_PAGE_RAIN_x,0);//ПЕРЕМЕННАЯ ВРЕМЯ МЕЖДУ ВКЛ ДВОРНИКА
	if (addr_canal_2 == 0xFF)
	{
		addr_canal_2=1;
		pulse_time_canal_2=7;
		pause_time_canal_2=10;
		work_preset_canal_2=59;
		tmp_preset_canal_2=58;
		time_rain=9;
	}	// задаем по умолчанию
	pause_time_canal_2 = pause_time_canal_2 * pause_mul;	// преобразование величин
	pulse_time_canal_2 = pulse_time_canal_2 + time_rotate;	// преобразование величин
	time_rain = time_rain * 60;	// преобразование величин
}
//================================================================================================================
void init_uart2()
{
	init_uart2_tx(APBCLK, 9600);
	init_EXTI_mif();
	init_TIMER17(0x960);	// 9600
}
//================================================================================================================
void init_mif(void)
{
	RCC_Configuration_HSE48();
	//		RCC_Configuration_HSI48();
	init_gpio_mif();
	init_uart(APBCLK, 57600);
	flash_unlock();
	mif_loud_var_from_flash();	// ЧТЕНИЕ ПЕРЕМЕННЫХ
	init_uart2();
	flash_lock();
	//==================================================================================================================================================================
	SysTick_Config(APBCLK / 5000);	//	100 мkс.//SysTick_Config(SystemFrequency / 1000);	// Настройка SysTick - 1 мс.
	//==================================================================================================================================================================
}
//======================================================================================================================================================================
void  output_485 (void)
{
	// ВКЛ ПЕРЕДАТЧИКА 485
	setout_485;
}
//======================================================================================================================================================================
void  input_485 (void)
{
	// ВЫКЛ ПЕРЕДАТЧИКА 485
	Delay_ms(20);
	setin_485;
}
//===================================================================//ЧТЕНИЕ СОСТОЯНИЯ МОДУЛЯ
void get_sost ( void )
{
	uart1_32(0x000BBBAA);
	uart1_32(0x00310000);
	uart1(rain);
	uart1(water);
	uart1(manual);
	uart1(myaddr);
	uart1(0);
	uart1(0);
	uart1(0x77);
	output_485();

	uart2_32(0x000BBBAA);
	uart2_32(0x00310000);
	uart2(rain);
	uart2(water);
	uart2(manual);
	uart2(myaddr);
	uart2(0);
	uart2(0);
	uart2(0x77);
	input_485();
}
//======================================================================================================================================================================
unsigned char temp_save = 0; // переменная для сохранения настроек

char mif_command (char command)
{
	extern char Buffer_in [];
	char canal_2;
	long t_pause_time_canal_2, t_pulse_time_canal_2, t_time_rain;

	if (command == 0x31)	// ЧТЕНИЕ СОСТОЯНИЕ МОДУЛЯ
	{
		if (Buffer_in [0] == myaddr)
		{
			shadow = Buffer_in[0x04] & 1;
			get_sost();
		}
		return 0;
	}

	//----------------------------------------------------------				WRITE PARAM
	if ((command == 0x34)&&(temp_save == 1)) // КОМАНДА ЗАПИСАТЬ ПАРАМЕТРЫ МОДУЛЯ
	{
		if (Buffer_in [0] == myaddr)
		{
			//__disable_irq ();
			write_var(FLASH_PAGE_RAIN_x,0,Buffer_in32(0x04));
			write_var(FLASH_PAGE_CAN2_Timers,0,Buffer_in32(0x08));
			write_var(FLASH_PAGE_CAN2_Timers,1,Buffer_in32(0x0C));
			write_var(FLASH_PAGE_CAN2_ADDR_T,0,Buffer_in[0x10]);
			write_var(FLASH_PAGE_CAN2_Preset,0,Buffer_in[0x14]);
			write_var(FLASH_PAGE_CAN2_Preset,1,Buffer_in[0x18]);
			temp_save = 0;
			mif_loud_var_from_flash();
			GPIOB->ODR |= GPIO_BRR_BR_1;
			Delay_ms(3000);
		}
		return 0;
	}
	//-----------------------------------------------------------
	if ((command == 0x42) && (want_canal_2 == 0))	// ТЕСТ МОДУЛЯ
	{
		if (Buffer_in [0] == myaddr)
		{
			tmp_pause_time_canal_2 = 3;
		}
		return 0;
	}

	//-----------------------------------------------------------//			READ_ALL_PARAM
	if ((command == 0x34)&&(temp_save==0))	// КОМАНДА СЧИТАТЬ ПАРАМЕТРЫ МОДУЛЯ
	{
		if (Buffer_in [0] == myaddr)
		{
			temp_save = 1;

			t_pause_time_canal_2 = pause_time_canal_2;
			t_pulse_time_canal_2 = pulse_time_canal_2;
			t_time_rain = time_rain;
			t_pause_time_canal_2 = t_pause_time_canal_2 / pause_mul;//преобразование величин
			t_pulse_time_canal_2 = t_pulse_time_canal_2-time_rotate;//преобразование величин
			t_time_rain = t_time_rain / 60;//преобразование величин
			uart1_32(0x001CBBAA); uart1_32(0x00340000);//1234
			uart1_32(t_time_rain);			// 5678				8
			uart1_32(t_pulse_time_canal_2);	// 9abc				C
			uart1_32(t_pause_time_canal_2);	// def10				10
			uart1_32(addr_canal_2);			// 11 12 13 14		14
			uart1_32(work_preset_canal_2);	// 15 16 17 18		18
			uart1_32(tmp_preset_canal_2);	// 19 1A 1B 1C		1C
			output_485();
			uart2_32(0x001CBBAA);uart2_32(0x00340000);//1234
			uart2_32(t_time_rain);			// 5678
			uart2_32(t_pulse_time_canal_2);	// 9abc
			uart2_32(t_pause_time_canal_2);	// def10
			uart2_32(addr_canal_2);			// 11 12 13 14
			uart2_32(work_preset_canal_2);	// 15 16 17 18
			uart2_32(tmp_preset_canal_2);	// 19 1A 1B 1C
			input_485();
		}
		return 0;
	}

	return command;
}
//======================================================================================================================================================================
void mif_logic()
{
	////////////////////////////////////////////////////////////////////////////////////////// ЧТЕНИЕ АДРЕСА МОДУЛЯ
	if ((GPIOA->IDR & GPIO_Pin_7 ) == 0) 	myaddr |= 0x01; else myaddr &= 0xFE;
	if ((GPIOA->IDR & GPIO_Pin_10 ) == 0) 	myaddr |= 0x02; else myaddr &= 0xFD;
	if ((GPIOA->IDR & GPIO_Pin_9 ) == 0) 	myaddr |= 0x04; else myaddr &= 0xFB;
	if ((GPIOA->IDR & GPIO_Pin_14 ) == 0) 	myaddr |= 0x08; else myaddr &= 0xF7;
	if ((GPIOA->IDR & GPIO_Pin_13 ) == 0) 	myaddr |= 0x10; else myaddr &= 0xEF;
	//////////////////////////////////////////////////////////////////////////////////////////  ДЕМОНСТРАЦИОННЫЙ РЕЖИМ
	if ((myaddr == 0x1F) && (myaddr_tmp !=myaddr)) { pulse_time_canal_2 = 14;pause_time_canal_2 = 5*60;tmp_pause_time_canal_2 = 3;}// 7cek 1min
	if ((myaddr_tmp == 0x1F) && (myaddr_tmp !=myaddr)) mif_loud_var_from_flash();      // ПЕРЕХОД В РАБОЧИЙ РЕЖИМ ИЗ ДЕМО
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if ((GPIOA->IDR & GPIO_Pin_6 ) == 0) rain = 0; else rain = 1;	// ЧТЕНИЕ ДАТЧИКА ДОЖДЯ
	if ((GPIOA->IDR & GPIO_Pin_1 ) == 0) water = 0; else water = 1;	// ЧТЕНИЕ ДАТЧИКА ВОДЫ

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//----------------------------				РАБОТА
	if  (tmp_pulse_time_canal_2 == 1) {tmp_pause_time_canal_2 = pause_time_canal_2; want_canal_2 = 0;}
	if  (tmp_pause_time_canal_2 == 1) {tmp_pulse_time_canal_2 = pulse_time_canal_2;	// если пришло время мыть
	if  (water==1) want_canal_2 = 1;} // если есть вода

	if   (tmp_pulse_time_canal_2 != 0)    								tmp_pulse_time_canal_2 --;// ДЕКРЕМЕНТ ВРЕМЕНИ
	if  ((tmp_pause_time_canal_2 != 0) && (pulse_time_canal_2 != 0)) 	tmp_pause_time_canal_2 --;// ДЕКРЕМЕНТ ВРЕМЕНИ
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//						timer время между поворотом камеры и включением насоса
	if (tmp_rotate_time_canal_2 == 1) on_canal_2;
	//
	if (tmp_rotate_time_canal_2 != 0) tmp_rotate_time_canal_2 --;
	//
	if (tmp_wiper_time_canal_2 != 0) tmp_wiper_time_canal_2 --;
	//

	/////////////////////////////////////////////////////////////// ПОМЫВКА ЗАКОНЧЕНА
	if ((tmp_want_canal_2 == 0) && (want_canal_2 == 1))
	{
		//					Pelco[1]=addr_canal_0;Pelco[3]=Clr_Preset;Pelco[5]=tmp_preset_canal_0;Out_Pelco_D(); //
		Camera_Save_position(tmp_preset_canal_2); // cохранить тек позицию
		Camera_Move_position(work_preset_canal_2);// поворачиваем камеру
		tmp_rotate_time_canal_2 = time_rotate+1;	tmp_wiper_time_canal_2 = 10+time_rotate;//	tmp_pomp_time_canal_2 = 0x09;	 // и запускаем таймер между поворотом камеры и включением насоса
	}

	if ((tmp_want_canal_2 == 1) && (want_canal_2 == 0))
	{
		off_canal_2;
		Camera_Move_position(tmp_preset_canal_2); // поворачиваем камеру обратно
		Camera_Wiper(); // дворник вкл еще раз
		Camera_Run_Tur_1();
	}

	if (rain) // если дождь
	{
		if (rain_tmp == 0)
		{
			tmp_time_rain = time_rain;
			Camera_Wiper();
			Camera_Run_Tur_1();
		} // если начался// дворник вкл
		if (tmp_time_rain -- == 0 )
		{
			tmp_time_rain = time_rain;
			Camera_Wiper();
			Camera_Run_Tur_1();
		} //если время// дворник вкл//
	}

	if (shadow != 0)
	{
		if ((water != water_tmp) | (rain != rain_tmp) | (manual != manual_tmp) | (myaddr != myaddr_tmp))
			get_sost();
	};// ЕСЛИ ВКЛЮЧЕНО СЛЕЖЕНИЕ ЗА СОСТОЯНИЕМ

	tmp_want_canal_2 = want_canal_2;// ОБНОВЛЕНИЕ ПЕРЕМЕННЫХ
	rain_tmp = rain;// ОБНОВЛЕНИЕ ПЕРЕМЕННЫХ
	water_tmp = water;// ОБНОВЛЕНИЕ ПЕРЕМЕННЫХ
	myaddr_tmp = myaddr;// ОБНОВЛЕНИЕ ПЕРЕМЕННЫХ
}

void mif_SysTick_Handler()
{
	if ((GPIOA->IDR & GPIO_Pin_5 ) == 0)
		manual = 1;
	else
		manual = 0;// ЧТЕНИЕ РУЧНОГО РЕЖИМА
	if ((manual_tmp == 0 ) && (manual == 1 ) && (want_canal_2 == 0))
	{
		tmp_pause_time_canal_2 = 3;
	} // BклЮЧЕНИЕ РУЧНОГО РЕЖИМА
	manual_tmp = manual;// ОБНОВЛЕНИЕ ПЕРЕМЕННЫХ

	if  (mif_time != 0)	// ДЕКРЕМЕНТ ВРЕМЕНИ
	{
		if	(mif_time_tmp -- == 0)
		{
			mif_time_tmp = mif_time;  //// ДЕКРЕМЕНТ ВРЕМЕНИ

			d2 ++;
			d2 &=0xF;	// 0 1 2 3 4 5 6 7 8 9 a b c d e f

			if ((d2 & 3) == 0)
				mif_logic(); // 1/4 ТАКТ ЛОГИКИ

			//	Delay_ms(50);
			// xor светика.
			if ((d2 & 2) == 0)
				GPIOB->ODR |= GPIO_BRR_BR_1;
			else
				GPIOB->ODR &= ~GPIO_BRR_BR_1;	// 1/2  РАБ РЕЖИМ СВЕТОДИОДА

			if ((water == 0) && ((d2 & 0xF) != 0xF))
				GPIOB->ODR |= GPIO_BRR_BR_1;	// РЕЖИМ СВЕТОДИОДА С ОТСУТСТВИЕМ ВОДЫ
		}
	}
}
