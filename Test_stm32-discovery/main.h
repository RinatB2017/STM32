//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
#ifndef MAIN_H
#define MAIN_H
//--------------------------------------------------------------------------------
#define	true	1
#define false	!true
#define	bool	char
//--------------------------------------------------------------------------------
uint8_t convert_ascii_to_value(uint8_t hi, uint8_t lo);
uint16_t crc16(uint8_t *pcBlock, uint8_t len);
uint8_t get_address(void);
void send_data(void);
void send_byte(uint8_t b);
void blink_ON(void);
void blink_OFF(void);
void pump_ON(void);
void pump_OFF(void);
void relay_ON(void);
void relay_OFF(void);
void wash(bool value);
bool check_LEVEL(void);
bool check_RAIN(void);
void Delay_ms(uint32_t ms);
void command(void);
void work(void);
void f_read(void);
void f_write(void);
void f_test(void);
void f_reset(void);
void camera_save_position (void);
void camera_move_position (void);
void camera_return (void);
void camera_wiper (void);
void camera_Run_Tur_1 (void);
void write_RS485(void);
void read_RS485(void);
void write_FLASH(void);
void read_FLASH(void);
//--------------------------------------------------------------------------------
#endif
