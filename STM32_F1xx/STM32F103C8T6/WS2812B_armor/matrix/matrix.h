#ifndef MATRIX_H
#define MATRIX_H
//------------------------------------------------------------
#include "bitmap.h"
//------------------------------------------------------------
#define bool	char
#define true	1
#define false	0
//------------------------------------------------------------
#define SIZE_MATRIX_0 20
#define SIZE_MATRIX_1 30
#define SIZE_MATRIX_2 36
#define SIZE_MATRIX_3 40
#define SIZE_MATRIX_4 36
#define SIZE_MATRIX_5 30
#define SIZE_MATRIX_6 25
#define SIZE_MATRIX_7 20
#define SIZE_MATRIX_8 25

#define MAX_ADDRESS_FRONT     (SIZE_MATRIX_0 + SIZE_MATRIX_1 + SIZE_MATRIX_6)
#define MAX_ADDRESS_SHOULDERS SIZE_MATRIX_6
#define MAX_ADDRESS_BACK      (SIZE_MATRIX_2 + (SIZE_MATRIX_3 / 2))

#define DEBUG_SIZE	6	//TODO тестовая полоска

#define MAX_SIZE	SIZE_MATRIX_0 + \
		SIZE_MATRIX_1 + \
		SIZE_MATRIX_2 + \
		SIZE_MATRIX_3 + \
		SIZE_MATRIX_4 + \
		SIZE_MATRIX_5 + \
		SIZE_MATRIX_6 + \
		SIZE_MATRIX_7 + \
		SIZE_MATRIX_8

extern unsigned int current_led;

extern unsigned int current_led_front;
extern unsigned int current_led_back;
extern unsigned int currrnt_led_shoulders;

extern int addr_0;
extern int addr_1;
extern int addr_2;
extern int addr_3;
extern int addr_4;
extern int addr_5;
extern int addr_6;
extern int addr_7;
extern int addr_8;

extern RGB_t *matrix0;
extern RGB_t *matrix1;
extern RGB_t *matrix2;
extern RGB_t *matrix3;
extern RGB_t *matrix4;
extern RGB_t *matrix5;
extern RGB_t *matrix5;
extern RGB_t *matrix6;
extern RGB_t *matrix7;
extern RGB_t *matrix8;

extern RGB_t led_buffer[MAX_SIZE];
extern int led_index;
extern uint8_t color;
//------------------------------------------------------------
void init_addr_matrix(void);
void init_matrix0(RGB_t led);
void init_matrix1(RGB_t led);
void init_matrix2(RGB_t led);
void init_matrix3(RGB_t led);
void init_matrix4(RGB_t led);
void init_matrix5(RGB_t led);
void init_matrix6(RGB_t led);
void init_matrix7(RGB_t led);
void init_matrix8(RGB_t led);

void init_leds(void);
void show_leds(void);
void clear_leds(void);
void send_debug_message(char * msg);

bool set_left_pixel(unsigned int addr, RGB_t color);
bool set_right_pixel(unsigned int addr, RGB_t color);
bool set_horizontal_pixel(unsigned int addr, RGB_t color);
void set_horizontal(RGB_t color);

void set_pixel_matrix_0(unsigned int addr, RGB_t color);
void set_pixel_matrix_1(unsigned int addr, RGB_t color);
void set_pixel_matrix_2(unsigned int addr, RGB_t color);
void set_pixel_matrix_3(unsigned int addr, RGB_t color);
void set_pixel_matrix_4(unsigned int addr, RGB_t color);
void set_pixel_matrix_5(unsigned int addr, RGB_t color);
void set_pixel_matrix_6(unsigned int addr, RGB_t color);
void set_pixel_matrix_7(unsigned int addr, RGB_t color);
void set_pixel_matrix_8(unsigned int addr, RGB_t color);

bool set_left_pixel_front(unsigned int addr, RGB_t color);
bool set_right_pixel_front(unsigned int addr, RGB_t color);
bool set_left_pixel_back(unsigned int addr, RGB_t color);
bool set_right_pixel_back(unsigned int addr, RGB_t color);
bool set_left_pixel_shoulders(unsigned int addr, RGB_t color);
bool set_right_pixel_shoulders(unsigned int addr, RGB_t color);

void pause(int value);

//------------------------------------------------------------
#endif
