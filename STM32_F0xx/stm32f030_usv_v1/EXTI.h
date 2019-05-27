void  init_EXTI(){
		RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // �������� ������������ ������ SYSCFG

// PORT INIT	========_akkord=========_akkord=========_akkord========_akkord=========
//EXTI->IMR  |= EXTI_IMR_MR8	;//; Прерывания от 8 ноги		_akkord
//EXTI->RTSR = EXTI_RTSR_TR8; //по фронту;						_akkord
//EXTI->FTSR |= EXTI_FTSR_TR8; //по спаду; 							_akkord
//EXTI->EXTI_PR	  = EXTI_PR_PR6;	 //Pending bit for line 7	

	
//		SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PA; // ������������� ����� ��1(������ ���� �� ����� stm32f0xx.h)
		SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PA; // ������������� ����� ��5(������ ���� �� ����� stm32f0xx.h)
//		EXTI->IMR = 0x0002; // �������� ����������
		EXTI->IMR = 1<<5; // �������� ����������
//		EXTI->RTSR = 0x0002; // �������� ������������ �� ������
		EXTI->RTSR = 1<<5; // �������� ������������ �� ������
		// ������������� NVIC ��� ������� ����������
//		NVIC_EnableIRQ(EXTI0_1_IRQn); // ��������� ���������� �� ����� EXTI0_1
		NVIC_EnableIRQ(EXTI4_15_IRQn); // ��������� ���������� �� ����� a5
}

//extern void EXTI9_5_IRQHandler_akkord(void);

//void EXTI9_5_IRQHandler(void){
//	if(EXTI->PR & 1<<8)	{
//	EXTI ->PR |= 0x08	;// очищаем флаг		странно было раньше и работало :)
//	EXTI ->PR |= 1<<8	;// сбрасываем событие записью "1" в нужный бит
// if (GPIOA->IDR & GPIO_IDR_IDR8) GPIOA->BSRR =GPIO_BSRR_BR2; else GPIOA->BSRR =GPIO_BSRR_BS2;
//	EXTI9_5_IRQHandler_akkord();
//	}
//	if(EXTI->PR & 1<<9)	{
//	EXTI ->PR |= 1<<9	;// сбрасываем событие записью "1" в нужный бит
//	EXTI9_5_IRQHandler_RFID_TKUD();	
//	EXTI9_5_IRQHandler_RFID_TKD();
//	EXTI9_5_IRQHandler_RFID_USV();
//	}
//}

//void EXTI0_1_IRQHandler (void){ // � ��������� ��������� ����������!
void EXTI4_15_IRQHandler (void){ //  ��������� ��������� ����������

   if(EXTI->PR & (1<<5)) {

	   //	EXTI9_5_IRQHandler_akkord();//      ������ ���-�� ���
	   GPIOA->ODR ^= GPIO_BRR_BR_1;
      EXTI->PR|=1<<5;  }//������� ����� ����������
}
	
	
	
