
#define  out_485 	GPIOB->ODR |= GPIO_BRR_BR_1	//
#define  in_485 	Delay_ms(20);GPIOB->ODR &= ~GPIO_BRR_BR_1	//

#define  gate_read_44 	0x44	//68			//Команда чтения портов ВВ
#define  gate_write_64 	0x64	//100			//Команда установки портов ВВ

//void Delay( uint32_t Val);
void Delay( uint32_t Val) {  for( ; Val != 0; Val--) 	{	__nop(); } }// функция задержки

short Buffer_in16 (char offs_buf)
{
	extern char Buffer_in [];
	return (Buffer_in [offs_buf]+(Buffer_in [offs_buf+1]*0x100));
}

long Buffer_in32 (char offs_buf)
{
	extern char Buffer_in [];
	return (Buffer_in [offs_buf] + Buffer_in [offs_buf+1] * 0x100 + Buffer_in [offs_buf+2] * 0x10000 + Buffer_in [offs_buf + 3] * 0x1000000 );
}

void Stop_mode (void)
{
	extern long out_xh5461_time;
	extern char stop_enable;
	extern char RFID_Carta;
	stop_enable = 0;
	out_xh5461_time = 100;
	off_seg1();off_seg2();off_seg3();off_seg4();
	Delay(1000);
	EXTI ->PR |= 1<<9	;// сбрасываем событие записью "1" в нужный бит
	PWR->CR &= ~PWR_CR_PDDS; //ReSet PDDS bit in Power Control register (PWR_CR)   //выбрали  stop
    __WFI();RFID_Carta = 0;stop_enable = 0;  //stop	
}
//////////////////////////////////////////////			Функции ВВ			////////////////////////////////////////////////////////
void Gate_read(void) {//Функция 

	extern char Buffer_in [];
	volatile long addr ;
	long	d;
	addr = ((Buffer_in[4])+((Buffer_in[5])*0x100)+((Buffer_in[6])*0x10000)+((Buffer_in[7])*0x1000000));

	d = (*(__IO long*) addr);
//	out_485;
#ifdef RK_canal
	uart2_32(0x000CBBAA);
	uart2_32(0x00440000);
	uart2_32(addr);
	uart2_32(d);
#endif
#ifdef MIF
	output_485();
	uart2_32(0x000CBBAA);
	uart2_32(0x00440000);
	uart2_32(addr);
	uart2_32(d);
	input_485();
#else
	uart1_32(0x000CBBAA);
	uart1_32(0x00440000);
	uart1_32(addr);
	uart1_32(d);
#endif
//	 in_485;

}
void Gate_write(void) {//Функция 

	extern char Buffer_in [];
	volatile long addr ;
	volatile long data ;
	long	d;
	
	addr = ((Buffer_in[4])+((Buffer_in[5])*0x100)+((Buffer_in[0x6])*0x10000)+((Buffer_in[0x7])*0x1000000));
	data = ((Buffer_in[8])+((Buffer_in[9])*0x100)+((Buffer_in[0xA])*0x10000)+((Buffer_in[0xB])*0x1000000));
	*(__IO long*)(addr) = data; //Пишем
	
	d = (*(__IO long*) addr);
//	out_485;
#ifdef RK_canal
		uart2_32(0x000CBBAA);
		uart2_32(0x00640000);
		uart2_32(addr);
		uart2_32(d);
#endif
#ifdef MIF
		output_485();
		uart2_32(0x000CBBAA);
		uart2_32(0x00640000);
		uart2_32(addr);
		uart2_32(d);
		input_485();
#else
		uart1_32(0x000CBBAA);
		uart1_32(0x00640000);
		uart1_32(addr);
		uart1_32(d);
#endif
//	 in_485;
}	


//; ============================== функции command BIPERa 	=========================================				
 char io_command (char command){ //
		if (command == gate_read_44) 	{Gate_read(); return 0;}				//	команда чтения регистров ВВ
		if (command == gate_write_64) 	{Gate_write();return 0;}				//	команда записи регистров ВВ
	//	if (command == 0x50) 	{uart1_32(0x0004BBAA);uart1_32(0x50505050);//__WFI();
	//		Stop_mode();uart1_32(0x0004BBAA);uart1_32(0x60666060);
	//		return 0;}				//
	//	if (command == 0x51) 	{uart1_32(0x0004BBAA);uart1_32(0x51515151);Delay(1000);//__WFI();
    //    SCB->SCR |=SCB_SCR_SLEEPDEEP; //
//				PWR->CR |=  PWR_CR_LPDS; //  
	//			PWR->CR |= PWR_CR_PDDS; //Set PDDS bit in Power Control register (PWR_CR)   //выбрали  sleep
    //__WFI();uart1_32(0x0004BBAA);uart1_32(0x01666161);  //stop
	//		return 0;}				//
  		return command;}
