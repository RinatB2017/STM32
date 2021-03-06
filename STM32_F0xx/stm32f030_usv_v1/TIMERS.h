///////////////////////////////////////////////////////////////		Настраиваем TIMER 2		/////////////////////////////////
///////////////////////////////////////////////////////////////		Настраиваем TIMER 3		/////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void  init_TIMER3(){//long psc){	// ������������� TIM3
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //��������� ������������ ������� TIM3
	NVIC_EnableIRQ(TIM3_IRQn); // ��������� ���������� �� TIM3
	TIM3->PSC = 47000; //  -������� ������������
	TIM3->ARR = 52000; //  - ������� ����������������  (�� �������� ����� ������� ����)
	TIM3->CCR1 = 0xFFFF;TIM3->CCR2 = 0xFFFF;TIM3->CCR3 = 0xFFFF;TIM3->CCR4 = 0xFFFF; //  -
	TIM3->DIER |= TIM_DIER_UIE; // ���������� ���������� �� ������������
	TIM3->CR1 |= TIM_CR1_ARPE;
//	TIM3->CR1 |= TIM_CR1_CEN; // ��������� �������
}

//extern void TIM4_answer_NiHao_rk(void);
//extern void TIM4_answer_NiHaoend_rk(void);

//===============================================================
//				���������� ���������� �� TIM3
//===============================================================
//void TIM3_IRQHandler(void)
//{//������� ���������� UEV
//	TIM3->SR &=~ TIM_SR_UIF; // ���������� UIF � ���� ���������� �� ������� ����������
//	GPIOA->ODR ^= GPIO_BRR_BR_1;
//}
//===============================================================

///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void  init_TIMER16(){//long psc){	// �������������
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE); //��������� ������������ �������
	NVIC_EnableIRQ(TIM16_IRQn); // ��������� ����������
	TIM16->PSC = 479119; //  -������� ������������
	TIM16->ARR = 210; //  - ������� ����������������  (�� �������� ����� ������� ����)
	TIM16->DIER |= TIM_DIER_UIE; // ���������� ���������� �� ������������
	TIM16->CR1 |= TIM_CR1_CEN; // ��������� ������������ �������
}

//extern void TIM4_answer_NiHao_rk(void);
//extern void TIM4_answer_NiHaoend_rk(void);

//===============================================================
//				���������� ���������� �� TIM16
//===============================================================
#ifndef VGA
void TIM16_IRQHandler(void)
{//������� ���������� UEV
	uart2_TX_IRQHandler();
//	GPIOA->ODR ^= GPIO_BRR_BR_1;
	TIM16->SR &=~ TIM_SR_UIF; // ���������� UIF � ���� ���������� �� ������� ����������
}
#endif
//===============================================================
///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void  init_TIMER17(long arr_){	// �������������
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE); //��������� ������������ �������
	NVIC_EnableIRQ(TIM17_IRQn); // ��������� ����������
//vga	TIM17->PSC = 300; //  -������� ������������
//RK	TIM17->PSC = 100; //  -������� ������������
	TIM17->ARR = arr_;//210; //  - ������� ����������������  (�� �������� ����� ������� ����)
	TIM17->DIER |= TIM_DIER_UIE; // ���������� ���������� �� ������������
#ifndef MIF
	TIM17->CR1 |= TIM_CR1_CEN; // ��������� ������������ ������� � ���������� ����� ������ ��������� �� ����� ���� �.�. �� ����������
#endif
}

//===============================================================
//#ifndef VGA
//void TIM17_IRQHandler(void){
//#ifdef RK_canal
//	RK_TIM17_IRQHandler();
//#endif
//	TIM17->SR &=~ TIM_SR_UIF; // ���������� UIF � ���� ���������� �� ������� ����������
//		}
//#endif


