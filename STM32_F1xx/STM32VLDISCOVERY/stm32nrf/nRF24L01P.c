#include "stm32f10x_RCC.h"
#include "stm32f10x_GPIO.h"
#include "stm32f10x_SPI.h"

#include "nRF24l01P.h"

unsigned char buf1[4] = {0x00,0x00,0x00, 0x00};

//Define the commands for operate the nRF24L01P
#define READ_nRF_REG    0x00  	// Command for read register
#define WRITE_nRF_REG   0x20 	// Command for read register
#define RD_RX_PLOAD     0x61  	// Command for read Rx payload
#define WR_TX_PLOAD     0xA0  	// Command for write Tx payload
#define FLUSH_TX        0xE1 	// Command for flush Tx FIFO
#define FLUSH_RX        0xE2  	// Command for flush Rx FIFO
#define REUSE_TX_PL     0xE3  	// Command for reuse Tx payload
#define NOP             0xFF  	// Reserve

//Define the register address for nRF24L01P
#define CONFIG          0x00  //  Configurate the status of transceiver, mode of CRC and the replay of transceiver status
#define EN_AA           0x01  //  Enable the atuo-ack in all channels
#define EN_RXADDR       0x02  //  Enable Rx Address
#define SETUP_AW        0x03  // Configurate the address width
#define SETUP_RETR      0x04  //  setup the retransmit
#define RF_CH           0x05  // Configurate the RF frequency
#define RF_SETUP        0x06  // Setup the rate of data, and transmit power
#define NRFRegSTATUS    0x07  //
#define OBSERVE_TX      0x08  //
#define CD              0x09  //    //Carrier detect
#define RX_ADDR_P0      0x0A  // Receive address of channel 0
#define RX_ADDR_P1      0x0B  // Receive address of channel 1
#define RX_ADDR_P2      0x0C  // Receive address of channel 2
#define RX_ADDR_P3      0x0D  // Receive address of channel 3
#define RX_ADDR_P4      0x0E  // Receive address of channel 4
#define RX_ADDR_P5      0x0F  // Receive address of channel 5
#define TX_ADDR         0x10  //       Transmit address
#define RX_PW_P0        0x11  //  Size of receive data in channel 0
#define RX_PW_P1        0x12  //  Size of receive data in channel 1
#define RX_PW_P2        0x13  //  Size of receive data in channel 2
#define RX_PW_P3        0x14  //  Size of receive data in channel 3
#define RX_PW_P4        0x15  // Size of receive data in channel 4
#define RX_PW_P5        0x16  //  Size of receive data in channel 5
#define FIFO_STATUS     0x17  // FIFO Status

#define MAX_TX  	0x10  //Максимальное число передач прерывается
#define TX_OK   	0x20  //TX передают полное прерывание
#define RX_OK   	0x40  //Прерывание при приеме данных
///**************************************************************************************

//define the private constants in this library
//#define TX_ADR_WIDTH 3
//#define RX_ADR_WIDTH 5

unsigned char TxBuf[Buffer_Size] = {0};
unsigned char RxBuf[Buffer_Size] = {0};

unsigned char nRF24L01_Freq = 80;
unsigned char nRF24L01_power_rate = 0;

//define the initial Address
unsigned char  TX_ADDRESS[ADR_WIDTH]= {0x34,0x43,0x10,0x10,0x01};
unsigned char  RX_ADDRESS[ADR_WIDTH]= {0x34,0x43,0x10,0x10,0x01};

//Define the layer1:HW operation
unsigned char nRF24L01_SPI_Send_Byte(unsigned char dat);
void nRF24L01_HW_Init(void);
void nRF24L01_SPI_NSS_L(void);
void nRF24L01_SPI_NSS_H(void);
void nRF24L01_SPI_CE_L(void);
void nRF24L01_SPI_CE_H(void);

//Define the layer2:Reg operation
unsigned char SPI_WR_Reg(unsigned char reg, unsigned char value);
unsigned char SPI_Read_Buf(unsigned char reg, unsigned char *pBuf, unsigned char Len);
unsigned char SPI_Write_Buf(unsigned char reg, unsigned char *pBuf, unsigned char Len);
unsigned char SPI_Write_Buf_2(unsigned char reg, unsigned char *pBuf, unsigned char Len);
unsigned char SPI_RD_Reg(unsigned char reg);
unsigned char nRF24L01_SPI_RD_Byte(unsigned char dat);

//Define the layer3:application operation
/****************************************

All the functions is in "nRF24l01P.h"

 ****************************************/

//Define the other function
void nRF24L01_Delay_us(unsigned long n);

//Define the layer3 functions

void nRF24L01_Set_TX_Address(	unsigned char A,
		unsigned char B,
		unsigned char C,
		unsigned char D,
		unsigned char E)
{
	TX_ADDRESS[0] = A;
	TX_ADDRESS[1] = B;
	TX_ADDRESS[2] = C;
	TX_ADDRESS[3] = D;
	TX_ADDRESS[4] = E;
}
void nRF24L01_Set_RX_Address(	unsigned char A,
		unsigned char B,
		unsigned char C,
		unsigned char D,
		unsigned char E)
{
	RX_ADDRESS[0] = A;
	RX_ADDRESS[1] = B;
	RX_ADDRESS[2] = C;
	RX_ADDRESS[3] = D;
	RX_ADDRESS[4] = E;
}

unsigned char nRF24L01_Config(unsigned char freq, unsigned char power, unsigned char Rate)
{
	nRF24L01_Freq = 0;
	nRF24L01_power_rate = 0;

	if((freq>125)&&(freq<0))
		return 0;
	else
		nRF24L01_Freq = freq;

	if (P0dBm == power)
		nRF24L01_power_rate|=0x06;
	else if (Pm6dBm == power)
		nRF24L01_power_rate|=0x04;
	else if (Pm12dBm == power)
		nRF24L01_power_rate|=0x02;
	else if (Pm18dBm == power)
		nRF24L01_power_rate|=0x00;
	else 
		return 0;

	if (R2mbps == Rate)
	{nRF24L01_power_rate|=0x08;}
	else if (Rate == R1mbps)
	{nRF24L01_power_rate|=0x00;}
	else if (Rate == R250kbps)
		nRF24L01_power_rate|=0x20;
	else
		return 0;

	return 1;
}

void RX_Mode(void)
{
	unsigned char status=1, temp;
	nRF24L01_SPI_CE_H();

	SPI_WR_Reg(0x20, 0x00);  // освободить буфер передачи

	SPI_WR_Reg(0x27,0x70);

	nRF24L01_SPI_NSS_L();
	nRF24L01_SPI_Send_Byte(0xE2);
	nRF24L01_SPI_NSS_H();
	nRF24L01_Delay_us(1);

	nRF24L01_SPI_NSS_L();
	nRF24L01_SPI_Send_Byte(0xE1);
	nRF24L01_SPI_NSS_H();
	nRF24L01_Delay_us(1);

	nRF24L01_SPI_NSS_L();
	nRF24L01_SPI_Send_Byte(0xFF);
	nRF24L01_SPI_NSS_H();
	nRF24L01_Delay_us(1);

	SPI_WR_Reg(WRITE_nRF_REG + EN_RXADDR, 0x00); // 0x22 - 0x00

	SPI_WR_Reg(WRITE_nRF_REG + EN_AA, 0x00);  // 0x21 - 0x00

	// чтение регистра //
	temp = SPI_RD_Reg(0x02); //

	SPI_WR_Reg(WRITE_nRF_REG + EN_RXADDR, 0x01); // 0x22 - 0x00

	// чтение регистра //
	temp = SPI_RD_Reg(0x01); //

	SPI_WR_Reg(WRITE_nRF_REG + EN_AA, 0x00); // 0x22 - 0x00

	// чтение регистра //
	temp = SPI_RD_Reg(0x00); //

	SPI_WR_Reg(WRITE_nRF_REG + CONFIG , 0x0C); // 0x22 - 0x00


	SPI_WR_Reg(WRITE_nRF_REG + SETUP_RETR, 0x00);

	SPI_WR_Reg(WRITE_nRF_REG + SETUP_AW, 0x03);

	// чтение регистра //
	temp = SPI_RD_Reg(SETUP_AW); //

	SPI_Write_Buf(WRITE_nRF_REG + TX_ADDR, TX_ADDRESS, ADR_WIDTH);  // адресс

	// чтение регистра //
	temp = SPI_RD_Reg(SETUP_AW); //

	SPI_Write_Buf_2(WRITE_nRF_REG + TX_ADDR, TX_ADDRESS, ADR_WIDTH);  // адресс

	SPI_WR_Reg(WRITE_nRF_REG + RF_CH, 0x50); // канал и мощность

	// чтение регистра
	temp = SPI_RD_Reg(0x06);
	// запись регистра //
	SPI_WR_Reg(WRITE_nRF_REG+RF_SETUP, 0x07);

	// чтение регистра
	temp = SPI_RD_Reg(0x06);
	// запись регистра //
	SPI_WR_Reg(WRITE_nRF_REG+RF_SETUP, 0x07);

	// чтение регистра
	temp = SPI_RD_Reg(0x06);
	// запись регистра //
	SPI_WR_Reg(WRITE_nRF_REG+RF_SETUP, 0x07);
	// чтение регистра
	temp = SPI_RD_Reg(0x06);
	// запись регистра //
	SPI_WR_Reg(WRITE_nRF_REG+RF_SETUP, 0x07);
	// чтение регистра
	temp = SPI_RD_Reg(0x06);
	// запись регистра //
	SPI_WR_Reg(WRITE_nRF_REG+RF_SETUP, 0x07);

	// чтение регистра
	temp = SPI_RD_Reg(FEATURE);
	// запись регистра //

	SPI_WR_Reg(WRITE_nRF_REG + FEATURE, 0x04); /// регистр опций

	// запись регистра //
	SPI_WR_Reg(WRITE_nRF_REG + DYNPD, 0x01);

	// чтение регистра
	temp = SPI_RD_Reg(CONFIG);

	// запись регистра //
	SPI_WR_Reg(WRITE_nRF_REG + CONFIG, 0x0E);

	nRF24L01_SPI_CE_L();

	SPI_WR_Reg(WRITE_nRF_REG + STATUS, 0x70);

	nRF24L01_SPI_NSS_L();
	nRF24L01_SPI_Send_Byte(0xE2);
	nRF24L01_SPI_NSS_H();
	nRF24L01_Delay_us(1);

	nRF24L01_SPI_NSS_L();
	nRF24L01_SPI_Send_Byte(0xE1);
	nRF24L01_SPI_NSS_H();
	nRF24L01_Delay_us(1);

	nRF24L01_SPI_NSS_L();
	nRF24L01_SPI_Send_Byte(0xFF);
	nRF24L01_SPI_NSS_H();
	nRF24L01_Delay_us(1);

	// чтение регистра
	temp = SPI_RD_Reg(CONFIG);


	SPI_WR_Reg(WRITE_nRF_REG + CONFIG, 0x0F);

	nRF24L01_SPI_NSS_H();
	nRF24L01_SPI_CE_H();
}

void TX_Mode(void)
{
	unsigned char status=1, temp;

	nRF24L01_SPI_CE_L();

	SPI_WR_Reg(0x20, 0x0E);  // освободить буфер передачи

	SPI_WR_Reg(WRITE_nRF_REG + EN_AA, 0x00); 		// 0x21 00


	SPI_WR_Reg(WRITE_nRF_REG + EN_RXADDR, 0x01); 	// 0x22 010

	SPI_WR_Reg(WRITE_nRF_REG + SETUP_AW, 0x03);		// 0x23 03
	SPI_WR_Reg(WRITE_nRF_REG + SETUP_RETR, 0x00);	// x04 00
	SPI_WR_Reg(WRITE_nRF_REG + RF_CH, 0x50);		// 0x25 0x50 // канал и скорость передачи
	SPI_WR_Reg(WRITE_nRF_REG + RF_SETUP, 0x07);		// 0x06 07
	SPI_WR_Reg(WRITE_nRF_REG + DYNPD, 0x01);		// 0x3c 0x01

	temp = SPI_RD_Reg(0x06);

	SPI_WR_Reg(WRITE_nRF_REG + RF_SETUP, 0x07);

	temp = SPI_RD_Reg(0x06);
	SPI_WR_Reg(WRITE_nRF_REG + RF_SETUP, 0x07);
	nRF24L01_Delay_us(500);

	//	 SPI_WR_Reg(WRITE_nRF_REG , 0x0C);

	nRF24L01_SPI_CE_L();
}

unsigned char nRF24L01_TxPacket(unsigned char * tx_buf)
{
	unsigned char status;

	nRF24L01_SPI_CE_L();
	SPI_Write_Buf(WRITE_nRF_REG + RX_ADDR_P0, TX_ADDRESS, ADR_WIDTH);
	nRF24L01_SPI_CE_L();

	status=SPI_RD_Reg(NRFRegSTATUS);
	SPI_Write_Buf(WRITE_nRF_REG + RX_ADDR_P0, TX_ADDRESS, ADR_WIDTH);
	SPI_WR_Reg(WRITE_nRF_REG + CONFIG, 0x07);

	SPI_WR_Reg(FLUSH_TX,0xff);
	SPI_Write_Buf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH);
	SPI_WR_Reg(WRITE_nRF_REG + CONFIG, 0x0e);
	nRF24L01_SPI_CE_H();
	nRF24L01_Delay_us(35);

	nRF24L01_SPI_CE_L();

	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)!=0);//Ожидающие передачи для завершения  IRQ
	status=SPI_RD_Reg(NRFRegSTATUS);//Значение чтения регистра состояния

	SPI_WR_Reg(WRITE_nRF_REG+NRFRegSTATUS,status); //Флаг прерывания Clear TX_DS или MAX_RT
	if(status&MAX_TX)//Максимальное число повторных передач
	{
		SPI_WR_Reg(FLUSH_TX,0xff);//Очистить регистр TX FIFO
		return MAX_TX;
	}
	if(status&TX_OK)//Отправить завершена
	{
		return TX_OK;
	}
	return 0xff;//Другие причины, которые не удалось отправить
}

void print_data(uint8_t data)
{
	switch (data)
	{
	case 0x00: sendchar('0'); break;
	case 0x01: sendchar('1'); break;
	case 0x02: sendchar('2'); break;
	case 0x03: sendchar('3'); break;
	case 0x04: sendchar('4'); break;
	case 0x05: sendchar('5'); break;
	case 0x06: sendchar('6'); break;
	case 0x07: sendchar('7'); break;
	case 0x08: sendchar('8'); break;
	case 0x09: sendchar('9'); break;
	case 0x0A: sendchar('A'); break;
	case 0x0B: sendchar('B'); break;
	case 0x0C: sendchar('C'); break;
	case 0x0D: sendchar('D'); break;
	case 0x0E: sendchar('E'); break;
	case 0x0F: sendchar('F'); break;
	default: break;
	}
}

void print_data_to_ascii(uint8_t data)
{
	uint8_t hi = (data >> 4) & 0x0F;
	uint8_t lo = (data & 0x0F);

	print_data(hi);
	print_data(lo);
}

unsigned char nRF24L01_RxPacket(void)	//unsigned char* rx_buf[4])
{
	unsigned char flag=0, i;
	unsigned char status, temp;
	unsigned char rx_buf[6] ={0,0,0,0,0,0};
	unsigned long int calc=0;

	nRF24L01_SPI_CE_L();
	nRF24L01_Delay_us(5);

	temp = SPI_RD_Reg(STATUS);


	SPI_WR_Reg(WRITE_nRF_REG + STATUS,0x61);
	nRF24L01_Delay_us(5);

	nRF24L01_SPI_NSS_L();
	//nRF24L01_SPI_Send_Byte(0xFF);
	nRF24L01_SPI_NSS_H();
	nRF24L01_Delay_us(1);

	nRF24L01_SPI_NSS_L();
	//nRF24L01_SPI_Send_Byte(0xFF);
	nRF24L01_SPI_NSS_H();
	nRF24L01_Delay_us(1);

	// чтение регистра
	temp = SPI_RD_Reg(RF_SETUP);
	nRF24L01_Delay_us(1);
	// тут надо прочитать данные из буфера регистра //
	nRF24L01_SPI_NSS_L();
	nRF24L01_SPI_Send_Byte(0x61);  // команда запроса данных из буфера приемника

	// пакет из 6 байт  // 0xA9 0xC0 это идентификатор он всегда далее 4 байта номера карты
	rx_buf[0] = nRF24L01_SPI_RD_Byte(0);// 169
	rx_buf[1] = nRF24L01_SPI_RD_Byte(0);// 192
	rx_buf[2] = nRF24L01_SPI_RD_Byte(0);//
	rx_buf[3] = nRF24L01_SPI_RD_Byte(0);//
	rx_buf[4] = nRF24L01_SPI_RD_Byte(0);//
	rx_buf[5] = nRF24L01_SPI_RD_Byte(0);

	sendchar(':');
	print_data_to_ascii(rx_buf[1]);
	print_data_to_ascii(rx_buf[0]);
	print_data_to_ascii(rx_buf[5]);
	print_data_to_ascii(rx_buf[4]);
	print_data_to_ascii(rx_buf[3]);
	print_data_to_ascii(rx_buf[2]);
	sendchar('\n');

	nRF24L01_Delay_us(2);

	nRF24L01_SPI_NSS_H();
	nRF24L01_Delay_us(2);
	nRF24L01_SPI_NSS_L();
	nRF24L01_SPI_Send_Byte(0xFF);
	nRF24L01_SPI_NSS_H();
	nRF24L01_SPI_CE_H();

	nRF24L01_Delay_us(5);

	nRF24L01_Delay_us(80);

	return flag;
}

//Define the layer2 functions
unsigned char SPI_RD_Reg(unsigned char reg)
{
	unsigned char reg_val;

	nRF24L01_SPI_NSS_L();                // CSN low, initialize SPI communication...
	nRF24L01_SPI_Send_Byte(reg);            // Select register to read from..
	reg_val = nRF24L01_SPI_Send_Byte(0);    // ..then read register value
	nRF24L01_SPI_NSS_H();                // CSN high, terminate SPI communication

	return(reg_val);        // return register value
}

unsigned char SPI_WR_Reg(unsigned char reg, unsigned char value)
{
	unsigned char status;

	nRF24L01_SPI_NSS_L();                  // CSN low, init SPI transaction

	status = nRF24L01_SPI_Send_Byte(reg);// select register
	nRF24L01_SPI_Send_Byte(value);             // ..and write value to it..
	nRF24L01_SPI_NSS_H();                   // CSN high again
	nRF24L01_Delay_us(10);
	nRF24L01_SPI_NSS_L();

	return(status);            // return nRF24L01 status unsigned char
}

unsigned char SPI_Read_Buf(unsigned char reg, unsigned char *pBuf, unsigned char Len)
{
	unsigned int status,i;

	nRF24L01_SPI_NSS_L();                    		// Set CSN low, init SPI tranaction
	status = nRF24L01_SPI_Send_Byte(reg);       		// Select register to write to and read status unsigned char

	for(i=0;i<Len;i++)
	{
		pBuf[i] = nRF24L01_SPI_Send_Byte(0);
	}

	nRF24L01_SPI_NSS_H();

	return(status);                    // return nRF24L01 status unsigned char
}

unsigned char SPI_Write_Buf(unsigned char reg, unsigned char *pBuf, unsigned char Len)
{
	nRF24L01_SPI_NSS_L();

	unsigned int status,i;
	nRF24L01_SPI_Send_Byte(0x30);

	for(i=0; i<Len; i++) //
	{
		nRF24L01_SPI_Send_Byte(pBuf[i]);
		//pBuf ++;
	}
	//status = nRF24L01_SPI_Send_Byte(reg);
	nRF24L01_SPI_NSS_H();
}

unsigned char SPI_Write_Buf_2(unsigned char reg, unsigned char *pBuf, unsigned char Len)
{
	nRF24L01_SPI_NSS_L();

	unsigned int status,i;
	nRF24L01_SPI_Send_Byte(0x2A);

	for(i=0; i<Len; i++) //
	{
		nRF24L01_SPI_Send_Byte(pBuf[i]);
		//pBuf ++;
	}
	//status = nRF24L01_SPI_Send_Byte(reg);
	nRF24L01_SPI_NSS_H();
}


//Define the layer1 functions
unsigned char nRF24L01_SPI_Send_Byte(unsigned char dat)
{
	/* Loop while DR register in not emplty */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI2 peripheral */
	SPI_I2S_SendData(SPI1, dat);
	nRF24L01_Delay_us(10);

	/* Wait to receive a byte */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI1);
}

unsigned char nRF24L01_SPI_RD_Byte(unsigned char dat)
{
	/* Loop while DR register in not emplty */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI2 peripheral */
	SPI_I2S_SendData(SPI1, dat);
	nRF24L01_Delay_us(10);

	/* Wait to receive a byte */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI1);
}


// тут перенастроил //
void nRF24L01_SPI_NSS_H(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_2);
}

void nRF24L01_SPI_NSS_L(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_2);
}
void nRF24L01_SPI_CE_H(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_3);
}
void nRF24L01_SPI_CE_L(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_3);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void nRF24L01_HW_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	//								SCK			MOSI
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	//	MISO
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	/* SPI1 configuration */
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStruct.SPI_Direction= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStruct);

	//	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	/*CE CSN Initial*/
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	/*IRQ Initial*/
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE);
}

unsigned char NRF24L01_Check(void)
{
	unsigned char buf[5]={0xa9,0xa9,0xa9,0xa9,0xa9,0xa9,0xa9,0xa9,0xa9};
	unsigned char buf1[5];
	//	unsigned char status;
	unsigned char i;
	SPI_Read_Buf(TX_ADDR,buf1,5);
	for(i=0;i<9;i++)
		if(buf1[i]!=0xA9)
			break;
	if(i!=9)
		return 1;                               //NRF24L01І»ФЪО»
	return 0;		                                //NRF24L01ФЪО»
}

void nRF24L01_Delay_us(unsigned long n)
{
	unsigned long int i;

	for (i=0; i<n; i++)
	{

	}
}
