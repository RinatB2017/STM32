#define Set_Preset 3
#define Clr_Preset 5
#define Go_Preset 7
#define Run_Pattern 0x23

extern void send_byte(uint8_t data);
extern void read_RS485();
extern void write_RS485();

// структура посылки Pelco-D
/*

 //|  byte 1  |   byte 2   |  byte 3  |  byte 4  | byte 5 | byte 6 | byte 7 |
 ---------- ------------ ---------- ---------- -------- -------- --------
 //|Sync 0xFF | Cam Adress | Command1 | Command2 | Data 1 | Data 2 |Checksum|

*/
char Pelco[20] = { 0xFF, 0, 0, 0, 0, 0 }; //структура

//; =============================	подпрограмма вычислени€ контрольной суммы	==================================================
void xor_csum(char Buffer[], int lench)
{
	long n;
	char crc = 0; // инициализаци€ €чейки контрольной суммы
	for (n = 0; n < lench; n++)
	{
		crc = crc ^ Buffer[n];
	} // вычисление контрольной суммы
	Buffer[lench] = crc;
}
//======================================================================================================================================================================
void Out_Pelco_D(void)
{
//	Pelco [1] = 0x01;//Buffer_in [8];	// ADDR
	Pelco[2] = 0x00; //Buffer_in [9];	// com1
//	Pelco [3] = 0x07;//Buffer_in [10];	// com2
	Pelco[4] = 0x00; //Buffer_in [11];	// PAN
//	Pelco [5] = 0x01;//Buffer_in [12];	// TILT
	Pelco[6] = Pelco[1] ^ Pelco[2] ^ Pelco[3] ^ Pelco[4] ^ Pelco[5]; // вычисление контрольной суммы

	write_RS485();
	send_byte(0xFF);
	send_byte(Pelco[1]);
	send_byte(Pelco[2]);
	send_byte(Pelco[3]);
	send_byte(Pelco[4]);
	send_byte(Pelco[5]);
	send_byte(Pelco[6]);
	read_RS485();
}
//======================================================================================================================================================================
void Out_Pelco_D1(void) {
//	Pelco [1] = 0x01;//Buffer_in [8];	// ADDR
	Pelco[2] = 0x00; //Buffer_in [9];	// com1
//	Pelco [3] = 0x07;//Buffer_in [10];	// com2
	Pelco[4] = 0x00; //Buffer_in [11];	// PAN
//	Pelco [5] = 0x01;//Buffer_in [12];	// TILT
	Pelco[6] = Pelco[1] + Pelco[2] + Pelco[3] + Pelco[4] + Pelco[5]; // вычисление суммы

	write_RS485();
	send_byte(0xFF);
	send_byte(Pelco[1]);
	send_byte(Pelco[2]);
	send_byte(Pelco[3]);
	send_byte(Pelco[4]);
	send_byte(Pelco[5]);
	send_byte(Pelco[6]);
	read_RS485();
}
//===================================================================================================================================================================
void Camera_Save_position(char preset) {
	extern char addr_canal_2;

	Pelco[1] = addr_canal_2;
	Pelco[3] = Set_Preset;
	Pelco[5] = preset;
	Out_Pelco_D1();
	//Out_Pelco_D(); // cохранить тек позицию
}
//===================================================================================================================================================================
void Camera_Move_position(char preset) {
	extern char addr_canal_2;

	Pelco[1] = addr_canal_2;
	Pelco[3] = Go_Preset;
	Pelco[5] = preset;
	Out_Pelco_D1();
	//Out_Pelco_D(); // поворачиваем камеру
}
//===================================================================================================================================================================
void Camera_Wiper() {
	extern char addr_canal_2;

	Pelco[1] = addr_canal_2;
	Pelco[3] = Go_Preset;
	Pelco[5] = 63;
	Out_Pelco_D1();
	//Out_Pelco_D(); // дворник вкл
}
//===================================================================================================================================================================
void Camera_Run_Tur_1() {
	extern char addr_canal_2;

	Pelco[1] = addr_canal_2;
	Pelco[3] = Run_Pattern;
	Pelco[5] = 0;
	Out_Pelco_D1();
	//Out_Pelco_D(); // 0- tur 1 ;    1 - tur 2
}
//===================================================================================================================================================================
