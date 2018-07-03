//------------------------------------------------------------
//
//------------------------------------------------------------
#include "ws2812b.h"
//------------------------------------------------------------
#include "matrix.h"
//------------------------------------------------------------
//3.3V 12-255
uint8_t color = 50;
//------------------------------------------------------------
RGB_t led_buffer[MAX_SIZE];

int addr_0 = 0;
int addr_1 = 0;
int addr_2 = 0;
int addr_3 = 0;
int addr_4 = 0;
int addr_5 = 0;
int addr_6 = 0;
int addr_7 = 0;
int addr_8 = 0;

RGB_t *matrix0 = 0;
RGB_t *matrix1 = 0;
RGB_t *matrix2 = 0;
RGB_t *matrix3 = 0;
RGB_t *matrix4 = 0;
RGB_t *matrix5 = 0;
RGB_t *matrix6 = 0;
RGB_t *matrix7 = 0;
RGB_t *matrix8 = 0;

int led_index = 0;
//------------------------------------------------------------
void clear_leds(void)
{
	int n = 0;

	RGB_t led;
	led.r = 0;
	led.g = 0;
	led.b = 0;
	for(n=0; n<MAX_SIZE; n++)
	{
		led_buffer[n] = led;
	}
}
//------------------------------------------------------------
void init_addr_matrix(void)
{
	addr_0 = 0;
	addr_1 = addr_0 + SIZE_MATRIX_0;
	addr_2 = addr_1 + SIZE_MATRIX_1;
	addr_3 = addr_2 + SIZE_MATRIX_2;
	addr_4 = addr_3 + SIZE_MATRIX_3;
	addr_5 = addr_4 + SIZE_MATRIX_4;
	addr_6 = addr_5 + SIZE_MATRIX_5;
	addr_7 = addr_6 + SIZE_MATRIX_6;
	addr_8 = addr_7 + SIZE_MATRIX_7;

	matrix0 = (RGB_t *)&led_buffer[addr_0];
	matrix1 = (RGB_t *)&led_buffer[addr_1];
	matrix2 = (RGB_t *)&led_buffer[addr_2];
	matrix3 = (RGB_t *)&led_buffer[addr_3];
	matrix4 = (RGB_t *)&led_buffer[addr_4];
	matrix5 = (RGB_t *)&led_buffer[addr_5];
	matrix6 = (RGB_t *)&led_buffer[addr_6];
	matrix7 = (RGB_t *)&led_buffer[addr_7];
	matrix8 = (RGB_t *)&led_buffer[addr_8];

	RGB_t led_r = { color, 0, 0 };
	RGB_t led_g = { 0, color, 0 };
	RGB_t led_b = { 0, 0, color };
	RGB_t led_0 = { 0, 0, 0 };

	init_matrix0(led_r);
	init_matrix1(led_g);
	init_matrix2(led_b);
	init_matrix3(led_r);
	init_matrix4(led_g);
	init_matrix5(led_b);
	init_matrix6(led_r);
	init_matrix7(led_g);
	init_matrix8(led_b);
}
//------------------------------------------------------------
void init_matrix0(RGB_t led)
{
	int n = 0;
	for(n=0; n<SIZE_MATRIX_0; n++)
	{
		matrix0[n] = led;
	}
}
//------------------------------------------------------------
void init_matrix1(RGB_t led)
{
	int n = 0;
	for(n=0; n<SIZE_MATRIX_1; n++)
	{
		matrix1[n] = led;
	}
}
//------------------------------------------------------------
void init_matrix2(RGB_t led)
{
	int n = 0;
	for(n=0; n<SIZE_MATRIX_2; n++)
	{
		matrix2[n] = led;
	}
}
//------------------------------------------------------------
void init_matrix3(RGB_t led)
{
	int n = 0;
	for(n=0; n<SIZE_MATRIX_3; n++)
	{
		matrix3[n] = led;
	}
}
//------------------------------------------------------------
void init_matrix4(RGB_t led)
{
	int n = 0;
	for(n=0; n<SIZE_MATRIX_4; n++)
	{
		matrix4[n] = led;
	}
}
//------------------------------------------------------------
void init_matrix5(RGB_t led)
{
	int n = 0;
	for(n=0; n<SIZE_MATRIX_5; n++)
	{
		matrix5[n] = led;
	}
}
//------------------------------------------------------------
void init_matrix6(RGB_t led)
{
	int n = 0;
	for(n=0; n<SIZE_MATRIX_6; n++)
	{
		matrix6[n] = led;
	}
}
//------------------------------------------------------------
void init_matrix7(RGB_t led)
{
	int n = 0;
	for(n=0; n<SIZE_MATRIX_7; n++)
	{
		matrix7[n] = led;
	}
}
//------------------------------------------------------------
void init_matrix8(RGB_t led)
{
	int n = 0;
	for(n=0; n<SIZE_MATRIX_8; n++)
	{
		matrix8[n] = led;
	}
}
//------------------------------------------------------------
