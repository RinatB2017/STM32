#ifndef MATRIX_H
#define MATRIX_H
//------------------------------------------------------------
#include "bitmap.h"
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
//------------------------------------------------------------
void clear_leds(void);
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
//------------------------------------------------------------
#endif
