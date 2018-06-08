//#define FLASH_PAGE_CAN0_Timers 7
//#define FLASH_PAGE_CAN1_Timers 8
#define FLASH_PAGE_CAN2_Timers  11 // 0x0B
#define FLASH_PAGE_CAN2_Preset 12 // 0x0C
#define FLASH_PAGE_ADDR_GATE 13 // 0x0D
#define FLASH_PAGE_RAIN_x 14 // 0x0E
#define FLASH_PAGE_CAN2_ADDR_T 15 // 0x0F

#define RKCONF_SHADOW 63 // 0x3F
/////////////////////////////////////////////////////   FLASH		///////////////////////////////////////////////////////////
long d0,d1;

#define FLASH_KEY1 ((uint32_t)0x45670123)
#define FLASH_KEY2 ((uint32_t)0xCDEF89AB) 

void flash_unlock(void)
{
	FLASH->KEYR = FLASH_KEY1;// разблокировка флеш
	FLASH->KEYR = FLASH_KEY2;//// разблокировка флеш
}

void flash_lock()
{
	FLASH->CR |= FLASH_CR_LOCK;
}// блокировка флеш

//Функция возврщает true когда можно стирать или писать память. 
char flash_ready(void)
{
	return !(FLASH->SR & FLASH_SR_BSY);
}
		
void flash_erase_page(long page)
{
	while (FLASH->SR & FLASH_SR_BSY);
	if (FLASH->SR & FLASH_SR_EOP)
	{
		FLASH->SR = FLASH_SR_EOP;
	}
	FLASH->CR|= FLASH_CR_PER; //Устанавливаем бит стирания одной страницы
	FLASH->AR = ((page*1024)+0x08000000); // Задаем её адрес
	FLASH->CR|= FLASH_CR_STRT; // Запускаем стирание
	while (!(FLASH->SR & FLASH_SR_EOP));//	while(!flash_ready());  //Ждем пока страница сотрется.
	FLASH->SR = FLASH_SR_EOP;
	FLASH->CR&= ~FLASH_CR_PER; //Сбрасываем бит обратно
}
//Прочитать флеш память		
long flash_read_page(long page,long address)
{
	return (*(__IO long*) (address+(page*1024)+0x08000000));
}//Прочитать флеш память

//Функция записи во флеш,
//data - указатель на записываемые данные
//page - адрес во flash
//count - количество записываемых байт, должно быть кратно 2
void Internal_Flash_Write (long page,char* data,unsigned int count)
{
	unsigned int i;
		
	while (FLASH->SR & FLASH_SR_BSY);		
		FLASH->CR |= FLASH_CR_PG; //Разрешаем программирование флеша   

	for (i = 0; i < count; i += 2)
	{
		*(__IO short*)(i+(page*1024)+0x08000000) = (((unsigned short)data[i + 1]) << 8) + data[i]; //Пишем 2 бата   
		while (!(FLASH->SR & FLASH_SR_EOP));
		FLASH->SR = FLASH_SR_EOP;
	}
	FLASH->CR &= ~(FLASH_CR_PG); //программируем флеш
}

//Прочитать флеш память		
long flash_read(long address)
{
	return (*(__IO long*) address);
}
		
//Функция записи во флеш, записываются 4 байта данных по заданному адресу:
void flash_write(long address,long data)
{
	FLASH->CR |= FLASH_CR_PG; //Разрешаем программирование флеша
	while(!flash_ready()) //Ожидаем готовности флеша к записи     ;
		*(__IO short*)address = (short)data; //Пишем младшие 2 бата
	while(!flash_ready()) ;
	address+=2;   data>>=16;
	*(__IO short*)address = (short)data; //Пишем старшие 2 байта
	while(!flash_ready())     ;
	FLASH->CR &= ~(FLASH_CR_PG); //Запрещаем программирование флеша
}
	
//// // определяем адрес (возвращат адрес первой ячейки со значением 0xFFFFFFFF )
long find_addr(long page,char var)
{	// var = 0(inc) или 1(dec)
	long addr;
		if	((var & 1 ) == 0)
		{
			addr =0 ;					// четая    !!! lim max
			if ((*(__IO long*) ((page*1024)+0x08000000))!= 0xFFFFFFFF)
			{	//ячейка с 0 адресом должна быть проинициализирована
				while (((*(__IO long*) ((addr*4 )+(page*1024)+0x08000000))!= 0xFFFFFFFF) &&  addr<0xFE)
				{
					addr++;
				}
			}
		}
		else
		{
			addr = 0xFF;	// He четая !!! lim min
			if ((*(__IO long*) ((page*1024)+0x080003FC))!= 0xFFFFFFFF)
			{	//ячейка с FF адресом должна быть проинициализирована
				while (((*(__IO long*) ((addr*4 )+(page*1024)+0x08000000))!= 0xFFFFFFFF) &&  addr>1)
				{
					addr--;
				}
			}
		}

		return addr;
}

 //Прочитать 32 переменную из флеш //page - адрес page во flash //var - имя(адрес) переменной 0-1
long read_var(long page,char var)
{
	long addr;
		if	((var & 1 ) == 0)	{ addr =0 ;					// четая    !!! lim max
			if ((*(__IO long*) ((page*1024)+0x08000000))!= 0xFFFFFFFF){
				while (((*(__IO long*) ((addr*4 )+(page*1024)+0x08000000))!= 0xFFFFFFFF) &&  addr<0xFE){addr++;}addr--;}}
		else 									{ addr = 0xFE;	// He четая !!! lim min
			if ((*(__IO long*) ((page*1024)+0x080003FC))!= 0xFFFFFFFF){
				while (((*(__IO long*) ((addr*4 )+(page*1024)+0x08000000))!= 0xFFFFFFFF) &&  addr>1)   {addr--;}addr++;}}

//		if	((var & 1 ) == 0)	{ if (addr != 0 )			{ addr -- ;}}	// четая    !!! lim max
//		else 									{ if (addr != 0xFF )	{ addr ++ ;}}	// He четая !!! lim min						
		return (*(__IO long*) ((addr*4)+(page*1024)+0x08000000));
}//Прочитать флеш память

//Записать 32 переменную в флеш //page - адрес page во flash //var - имя(адрес) переменной 0-1
void wr_var(long page,char var,long data)
{ // тупо пишет уверен что все в поряке (проверка в write_var)
	long addr;
	addr = find_addr (page,var);// указывает на следущую ячейку
//		if	((var & 1 ) == 0) {	addr ++ ;}	// четая    !!! lim max
//										else	{ addr -- ;}	// He четая !!! lim min		
	while (FLASH->SR & FLASH_SR_BSY);
	FLASH->CR |= FLASH_CR_PG; //Разрешаем программирование флеша   
		*(__IO short*)((addr*4) + (page*1024)+0x08000000) = data ; //Пишем 2 младших байта   
		*(__IO short*)((addr*4)+2+(page*1024)+0x08000000) = data >> 16; //Пишем 2 старших байта   
		while (!(FLASH->SR & FLASH_SR_EOP));
		FLASH->SR = FLASH_SR_EOP;	 
		FLASH->CR &= ~(FLASH_CR_PG); //программируем флеш
		}
	
//Записать 32 переменную в флеш //page - адрес page во flash //var - имя(адрес) переменной 0-1
void write_var(long page,char var,long data)
{
	flash_unlock();
	long addr;
	char page_ful=0;
	addr = find_addr (page,var);// указывает на следущую ячейку со значением 0xFFFFFFFF
//	uart1_32(0x001CBBAA);uart1_32(0x00870000);
//	uart1_32(addr);uart1_32(read_var(page,var));

//	uart1_32(((addr*4)+(page*1024)+0x08000000));uart1_32((*(__IO long*) ((addr*4)+(page*1024)+0x08000000)));

	if	((var & 1 ) == 0)
	{
		addr = addr + 1;
		if (addr ==0xFE) page_ful = 1 ;
	}	// четая    !!! lim max
	else
	{
		addr = addr - 1;
		if (addr ==0x01) page_ful = 1 ;
	}// He четая !!! lim min

	uart1_32(((addr*4)+(page*1024)+0x08000000));
	uart1_32((*(__IO long*) ((addr*4)+(page*1024)+0x08000000)));

	if ((*(__IO long*) ((addr*4)+(page*1024)+0x08000000) != 0xFFFFFFFF) || ( page_ful == 1 ) )
	{
		//Сохранить в озу 2 переменных
		d0=read_var(page,0);
		d1=read_var(page,1);
		Delay_ms(40);
		flash_erase_page(page);
		Delay_ms(40);
		wr_var(page,0,d0);
		wr_var(page,1,d1);
	}
	wr_var(page,var,data);
	flash_lock();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
 //
long find_free(long page,char step)
{	// на выходе addr указывает на первую ячейку со значением 0xFFFFFFFF
	long addr=0;
	if ((*(__IO long*) ((page*1024)+0x08000000))!= 0xFFFFFFFF)
	{//
		while (((*(__IO long*) ((addr*4 )+(page*1024)+0x08000000))!= 0xFFFFFFFF) &&  addr<(0xFF-step)){addr=addr+step;}
	}
	return addr;
}// addr указывает на первую ячейку со значением 0xFFFFFFFF

//Заполнить(считать) массив из флеш из 8-ми 32-ух бит. переменных
void read_mas8(long page,long mas[8])
{
	long addr,n;
	addr = find_free(page,9);// addr указывает на первую ячейку со значением 0xFFFFFFFF
	if (addr)	addr = addr - 9;//последнее записанное значение
	for (n=0;n<8;n++)
	{
		mas[n]=(*(__IO long*) (((addr+n+1)*4)+(page*1024)+0x08000000));
	}
}//Прочитать флеш память
											
void write_mas8(long page,long mas[8])
{
	long addr,n;
	addr = find_free(page,9);// addr указывает на первую ячейку со значением 0xFFFFFFFF
	if ((addr+9)>(0xFF-9))
	{
		flash_erase_page(page);
		addr=0;
	}
	while (FLASH->SR & FLASH_SR_BSY);
	FLASH->CR |= FLASH_CR_PG; //Разрешаем программирование флеша 
	*(__IO short*)((addr*4) + (page*1024)+0x08000000) = 0x1980 ; //Пишем 2 младших байта (признак)  
	*(__IO short*)((addr*4)+2+(page*1024)+0x08000000) = 0x2016; //Пишем 2 старших байта   
	for (n=0;n<8;n++)
	{
		*(__IO short*)(((addr+n+1)*4) + (page*1024)+0x08000000) = mas[n] ; 		//Пишем 2 младших байта
		*(__IO short*)(((addr+n+1)*4)+2+(page*1024)+0x08000000) = mas[n] >> 16;	//Пишем 2 старших байта
	}
	while (!(FLASH->SR & FLASH_SR_EOP));
	FLASH->SR = FLASH_SR_EOP;
	FLASH->CR &= ~(FLASH_CR_PG); //программируем флеш
}
