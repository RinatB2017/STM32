void  init_EXTI(){
		RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Включили тактирование модуля SYSCFG

// PORT INIT	========_akkord=========_akkord=========_akkord========_akkord=========
//EXTI->IMR  |= EXTI_IMR_MR8	;//; РџСЂРµСЂС‹РІР°РЅРёСЏ РѕС‚ 8 РЅРѕРіРё		_akkord
//EXTI->RTSR = EXTI_RTSR_TR8; //РїРѕ С„СЂРѕРЅС‚Сѓ;						_akkord
//EXTI->FTSR |= EXTI_FTSR_TR8; //РїРѕ СЃРїР°РґСѓ; 							_akkord
//EXTI->EXTI_PR	  = EXTI_PR_PR6;	 //Pending bit for line 7	

	
//		SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PA; // Конфигурируем вывод РА1(макрос берём из файла stm32f0xx.h)
		SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PA; // Конфигурируем вывод РА5(макрос берём из файла stm32f0xx.h)
//		EXTI->IMR = 0x0002; // Разрешил прерывание
		EXTI->IMR = 1<<5; // Разрешил прерывание
//		EXTI->RTSR = 0x0002; // Разрешил срабатывание по фронту
		EXTI->RTSR = 1<<5; // Разрешил срабатывание по фронту
		// Конфигурируем NVIC для внешних прерываний
//		NVIC_EnableIRQ(EXTI0_1_IRQn); // Разрешаем прерывание на линии EXTI0_1
		NVIC_EnableIRQ(EXTI4_15_IRQn); // Разрешаем прерывание на линии a5
}

//extern void EXTI9_5_IRQHandler_akkord(void);

//void EXTI9_5_IRQHandler(void){
//	if(EXTI->PR & 1<<8)	{
//	EXTI ->PR |= 0x08	;// РѕС‡РёС‰Р°РµРј С„Р»Р°Рі		СЃС‚СЂР°РЅРЅРѕ Р±С‹Р»Рѕ СЂР°РЅСЊС€Рµ Рё СЂР°Р±РѕС‚Р°Р»Рѕ :)
//	EXTI ->PR |= 1<<8	;// СЃР±СЂР°СЃС‹РІР°РµРј СЃРѕР±С‹С‚РёРµ Р·Р°РїРёСЃСЊСЋ "1" РІ РЅСѓР¶РЅС‹Р№ Р±РёС‚
// if (GPIOA->IDR & GPIO_IDR_IDR8) GPIOA->BSRR =GPIO_BSRR_BR2; else GPIOA->BSRR =GPIO_BSRR_BS2;
//	EXTI9_5_IRQHandler_akkord();
//	}
//	if(EXTI->PR & 1<<9)	{
//	EXTI ->PR |= 1<<9	;// СЃР±СЂР°СЃС‹РІР°РµРј СЃРѕР±С‹С‚РёРµ Р·Р°РїРёСЃСЊСЋ "1" РІ РЅСѓР¶РЅС‹Р№ Р±РёС‚
//	EXTI9_5_IRQHandler_RFID_TKUD();	
//	EXTI9_5_IRQHandler_RFID_TKD();
//	EXTI9_5_IRQHandler_RFID_USV();
//	}
//}

//void EXTI0_1_IRQHandler (void){ // И процедура обработки прерывания!
void EXTI4_15_IRQHandler (void){ //  процедура обработки прерывания

   if(EXTI->PR & (1<<5)) {

	   //	EXTI9_5_IRQHandler_akkord();//      Делаем что-то там…
	   GPIOA->ODR ^= GPIO_BRR_BR_1;
      EXTI->PR|=1<<5;  }//Очистка флага прерывания
}
	
	
	
