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
	FLASH->KEYR = FLASH_KEY1;// ������������� ����
	FLASH->KEYR = FLASH_KEY2;//// ������������� ����
}

void flash_lock()
{
	FLASH->CR |= FLASH_CR_LOCK;
}// ���������� ����

//������� ��������� true ����� ����� ������� ��� ������ ������. 
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
	FLASH->CR|= FLASH_CR_PER; //������������� ��� �������� ����� ��������
	FLASH->AR = ((page*1024)+0x08000000); // ������ � �����
	FLASH->CR|= FLASH_CR_STRT; // ��������� ��������
	while (!(FLASH->SR & FLASH_SR_EOP));//	while(!flash_ready());  //���� ���� �������� ��������.
	FLASH->SR = FLASH_SR_EOP;
	FLASH->CR&= ~FLASH_CR_PER; //���������� ��� �������
}
//��������� ���� ������		
long flash_read_page(long page,long address)
{
	return (*(__IO long*) (address+(page*1024)+0x08000000));
}//��������� ���� ������

//������� ������ �� ����,
//data - ��������� �� ������������ ������
//page - ����� �� flash
//count - ���������� ������������ ����, ������ ���� ������ 2
void Internal_Flash_Write (long page,char* data,unsigned int count)
{
	unsigned int i;
		
	while (FLASH->SR & FLASH_SR_BSY);		
		FLASH->CR |= FLASH_CR_PG; //��������� ���������������� �����   

	for (i = 0; i < count; i += 2)
	{
		*(__IO short*)(i+(page*1024)+0x08000000) = (((unsigned short)data[i + 1]) << 8) + data[i]; //����� 2 ����   
		while (!(FLASH->SR & FLASH_SR_EOP));
		FLASH->SR = FLASH_SR_EOP;
	}
	FLASH->CR &= ~(FLASH_CR_PG); //������������� ����
}

//��������� ���� ������		
long flash_read(long address)
{
	return (*(__IO long*) address);
}
		
//������� ������ �� ����, ������������ 4 ����� ������ �� ��������� ������:
void flash_write(long address,long data)
{
	FLASH->CR |= FLASH_CR_PG; //��������� ���������������� �����
	while(!flash_ready()) //������� ���������� ����� � ������     ;
		*(__IO short*)address = (short)data; //����� ������� 2 ����
	while(!flash_ready()) ;
	address+=2;   data>>=16;
	*(__IO short*)address = (short)data; //����� ������� 2 �����
	while(!flash_ready())     ;
	FLASH->CR &= ~(FLASH_CR_PG); //��������� ���������������� �����
}
	
//// // ���������� ����� (��������� ����� ������ ������ �� ��������� 0xFFFFFFFF )
long find_addr(long page,char var)
{	// var = 0(inc) ��� 1(dec)
	long addr;
		if	((var & 1 ) == 0)
		{
			addr =0 ;					// �����    !!! lim max
			if ((*(__IO long*) ((page*1024)+0x08000000))!= 0xFFFFFFFF)
			{	//������ � 0 ������� ������ ���� �������������������
				while (((*(__IO long*) ((addr*4 )+(page*1024)+0x08000000))!= 0xFFFFFFFF) &&  addr<0xFE)
				{
					addr++;
				}
			}
		}
		else
		{
			addr = 0xFF;	// He ����� !!! lim min
			if ((*(__IO long*) ((page*1024)+0x080003FC))!= 0xFFFFFFFF)
			{	//������ � FF ������� ������ ���� �������������������
				while (((*(__IO long*) ((addr*4 )+(page*1024)+0x08000000))!= 0xFFFFFFFF) &&  addr>1)
				{
					addr--;
				}
			}
		}

		return addr;
}

 //��������� 32 ���������� �� ���� //page - ����� page �� flash //var - ���(�����) ���������� 0-1
long read_var(long page,char var)
{
	long addr;
		if	((var & 1 ) == 0)	{ addr =0 ;					// �����    !!! lim max
			if ((*(__IO long*) ((page*1024)+0x08000000))!= 0xFFFFFFFF){
				while (((*(__IO long*) ((addr*4 )+(page*1024)+0x08000000))!= 0xFFFFFFFF) &&  addr<0xFE){addr++;}addr--;}}
		else 									{ addr = 0xFE;	// He ����� !!! lim min
			if ((*(__IO long*) ((page*1024)+0x080003FC))!= 0xFFFFFFFF){
				while (((*(__IO long*) ((addr*4 )+(page*1024)+0x08000000))!= 0xFFFFFFFF) &&  addr>1)   {addr--;}addr++;}}

//		if	((var & 1 ) == 0)	{ if (addr != 0 )			{ addr -- ;}}	// �����    !!! lim max
//		else 									{ if (addr != 0xFF )	{ addr ++ ;}}	// He ����� !!! lim min						
		return (*(__IO long*) ((addr*4)+(page*1024)+0x08000000));
}//��������� ���� ������

//�������� 32 ���������� � ���� //page - ����� page �� flash //var - ���(�����) ���������� 0-1
void wr_var(long page,char var,long data)
{ // ���� ����� ������ ��� ��� � ������ (�������� � write_var)
	long addr;
	addr = find_addr (page,var);// ��������� �� �������� ������
//		if	((var & 1 ) == 0) {	addr ++ ;}	// �����    !!! lim max
//										else	{ addr -- ;}	// He ����� !!! lim min		
	while (FLASH->SR & FLASH_SR_BSY);
	FLASH->CR |= FLASH_CR_PG; //��������� ���������������� �����   
		*(__IO short*)((addr*4) + (page*1024)+0x08000000) = data ; //����� 2 ������� �����   
		*(__IO short*)((addr*4)+2+(page*1024)+0x08000000) = data >> 16; //����� 2 ������� �����   
		while (!(FLASH->SR & FLASH_SR_EOP));
		FLASH->SR = FLASH_SR_EOP;	 
		FLASH->CR &= ~(FLASH_CR_PG); //������������� ����
		}
	
//�������� 32 ���������� � ���� //page - ����� page �� flash //var - ���(�����) ���������� 0-1
void write_var(long page,char var,long data)
{
	flash_unlock();
	long addr;
	char page_ful=0;
	addr = find_addr (page,var);// ��������� �� �������� ������ �� ��������� 0xFFFFFFFF
//	uart1_32(0x001CBBAA);uart1_32(0x00870000);
//	uart1_32(addr);uart1_32(read_var(page,var));

//	uart1_32(((addr*4)+(page*1024)+0x08000000));uart1_32((*(__IO long*) ((addr*4)+(page*1024)+0x08000000)));

	if	((var & 1 ) == 0)
	{
		addr = addr + 1;
		if (addr ==0xFE) page_ful = 1 ;
	}	// �����    !!! lim max
	else
	{
		addr = addr - 1;
		if (addr ==0x01) page_ful = 1 ;
	}// He ����� !!! lim min

	uart1_32(((addr*4)+(page*1024)+0x08000000));
	uart1_32((*(__IO long*) ((addr*4)+(page*1024)+0x08000000)));

	if ((*(__IO long*) ((addr*4)+(page*1024)+0x08000000) != 0xFFFFFFFF) || ( page_ful == 1 ) )
	{
		//��������� � ��� 2 ����������
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
{	// �� ������ addr ��������� �� ������ ������ �� ��������� 0xFFFFFFFF
	long addr=0;
	if ((*(__IO long*) ((page*1024)+0x08000000))!= 0xFFFFFFFF)
	{//
		while (((*(__IO long*) ((addr*4 )+(page*1024)+0x08000000))!= 0xFFFFFFFF) &&  addr<(0xFF-step)){addr=addr+step;}
	}
	return addr;
}// addr ��������� �� ������ ������ �� ��������� 0xFFFFFFFF

//���������(�������) ������ �� ���� �� 8-�� 32-�� ���. ����������
void read_mas8(long page,long mas[8])
{
	long addr,n;
	addr = find_free(page,9);// addr ��������� �� ������ ������ �� ��������� 0xFFFFFFFF
	if (addr)	addr = addr - 9;//��������� ���������� ��������
	for (n=0;n<8;n++)
	{
		mas[n]=(*(__IO long*) (((addr+n+1)*4)+(page*1024)+0x08000000));
	}
}//��������� ���� ������
											
void write_mas8(long page,long mas[8])
{
	long addr,n;
	addr = find_free(page,9);// addr ��������� �� ������ ������ �� ��������� 0xFFFFFFFF
	if ((addr+9)>(0xFF-9))
	{
		flash_erase_page(page);
		addr=0;
	}
	while (FLASH->SR & FLASH_SR_BSY);
	FLASH->CR |= FLASH_CR_PG; //��������� ���������������� ����� 
	*(__IO short*)((addr*4) + (page*1024)+0x08000000) = 0x1980 ; //����� 2 ������� ����� (�������)  
	*(__IO short*)((addr*4)+2+(page*1024)+0x08000000) = 0x2016; //����� 2 ������� �����   
	for (n=0;n<8;n++)
	{
		*(__IO short*)(((addr+n+1)*4) + (page*1024)+0x08000000) = mas[n] ; 		//����� 2 ������� �����
		*(__IO short*)(((addr+n+1)*4)+2+(page*1024)+0x08000000) = mas[n] >> 16;	//����� 2 ������� �����
	}
	while (!(FLASH->SR & FLASH_SR_EOP));
	FLASH->SR = FLASH_SR_EOP;
	FLASH->CR &= ~(FLASH_CR_PG); //������������� ����
}
