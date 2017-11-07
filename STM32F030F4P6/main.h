//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
#ifndef MAIN_H
#define MAIN_H
//--------------------------------------------------------------------------------
#define bool	char
#define true	1
#define false	0
//--------------------------------------------------------------------------------
void f_test();
void f_reset();
void f_read();
void f_write();

void send_byte(uint8_t data);
void send_data();

void work();
void command();

void camera_save_position (void);
void camera_move_position (void);
void camera_return (void);
void camera_wiper (void);
void camera_Run_Tur_1 (void);

uint8_t convert_ascii_to_value(uint8_t hi, uint8_t lo);
void convert_data_to_ascii(uint8_t data, uint8_t *hi_str, uint8_t *lo_str);
//--------------------------------------------------------------------------------
#endif
