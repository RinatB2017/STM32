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

unsigned int current_led = 0;    // текущий светодиод

unsigned int current_led_front = 0;
unsigned int current_led_back = 0;
unsigned int currrnt_led_shoulders = 0;

extern void delay_ms(uint32_t ms);
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
void init_leds(void)
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
void show_leds(void)
{
	while (!ws2812b_IsReady());
	ws2812b_SendRGB(led_buffer, MAX_SIZE);
}
//------------------------------------------------------------
void send_debug_message(char *msg)
{
	USARTSend(msg);
}
//------------------------------------------------------------
bool set_left_pixel(unsigned int addr, RGB_t color)
{
    unsigned int addr1 = SIZE_MATRIX_7;
    unsigned int addr2 = addr1 + SIZE_MATRIX_1;
    unsigned int addr3 = addr2 + SIZE_MATRIX_6;
    unsigned int addr4 = addr3 + SIZE_MATRIX_2;

    if (addr < addr1)
    {
        set_pixel_matrix_0(addr, color);
        return true;
    }
    if (addr < addr2)
    {
        set_pixel_matrix_5(addr - addr1, color);
        return true;
    }
    if (addr < addr3)
    {
        set_pixel_matrix_8(addr - addr2, color);
        return true;
    }
    if (addr < addr4)
    {
        set_pixel_matrix_4(addr - addr3, color);
        return true;
    }
    if (addr < MAX_SIZE)
    {
        return true;
    }

    return false;
}
//------------------------------------------------------------
bool set_right_pixel(unsigned int addr, RGB_t color)
{
    unsigned int addr1 = SIZE_MATRIX_0;
    unsigned int addr2 = addr1 + SIZE_MATRIX_5;
    unsigned int addr3 = addr2 + SIZE_MATRIX_8;
    unsigned int addr4 = addr3 + SIZE_MATRIX_4;

    if (addr < addr1)
    {
        set_pixel_matrix_7(addr, color);
        return true;
    }
    if (addr < addr2)
    {
        set_pixel_matrix_1(addr - addr1, color);
        return true;
    }
    if (addr < addr3)
    {
        set_pixel_matrix_6(addr - addr2, color);
        return true;
    }
    if (addr < addr4)
    {
        set_pixel_matrix_2(addr - addr3, color);
        return true;
    }
    if (addr < MAX_SIZE)
    {
        return true;
    }

    return false;
}
//------------------------------------------------------------
bool set_horizontal_pixel(unsigned int addr, RGB_t color)
{
    if (addr > SIZE_MATRIX_3)
    {
        return false;
    }
    set_pixel_matrix_3(addr, color);
    return true;
}
//------------------------------------------------------------
void set_horizontal(RGB_t color)
{
	int x = 0;
    for (x = 0; x < SIZE_MATRIX_3; x++)
    {
        set_pixel_matrix_3(x, color);
    }
}
//------------------------------------------------------------
void set_pixel_matrix_0(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_0)
    {
        matrix0[addr] = color;
    }
    else
    {
    	char err_str[50];
    	sprintf(err_str, "bad_addr_0 %d\n", addr);
        send_debug_message(err_str);
    }
}
//------------------------------------------------------------
void set_pixel_matrix_1(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_1)
    {
        matrix1[addr] = color;
    }
    else
    {
    	char err_str[50];
    	sprintf(err_str, "bad_addr_1 %d\n", addr);
        send_debug_message(err_str);
    }
}
//------------------------------------------------------------
void set_pixel_matrix_2(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_2)
    {
        matrix2[addr] = color;
    }
    else
    {
    	char err_str[50];
    	sprintf(err_str, "bad_addr_2 %d\n", addr);
        send_debug_message(err_str);
    }
}
//------------------------------------------------------------
void set_pixel_matrix_3(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_3)
    {
        matrix3[addr] = color;
    }
    else
    {
    	char err_str[50];
    	sprintf(err_str, "bad_addr_3 %d\n", addr);
        send_debug_message(err_str);
    }
}
//------------------------------------------------------------
void set_pixel_matrix_4(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_4)
    {
        matrix4[addr] = color;
    }
    else
    {
    	char err_str[50];
    	sprintf(err_str, "bad_addr_4 %d\n", addr);
        send_debug_message(err_str);
    }
}
//------------------------------------------------------------
void set_pixel_matrix_5(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_5)
    {
        matrix5[addr] = color;
    }
    else
    {
    	char err_str[50];
    	sprintf(err_str, "bad_addr_5 %d\n", addr);
        send_debug_message(err_str);
    }
}
//------------------------------------------------------------
void set_pixel_matrix_6(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_6)
    {
        matrix6[addr] = color;
    }
    else
    {
    	char err_str[50];
    	sprintf(err_str, "bad_addr_6 %d\n", addr);
        send_debug_message(err_str);
    }
}
//------------------------------------------------------------
void set_pixel_matrix_7(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_7)
    {
        matrix7[addr] = color;
    }
    else
    {
    	char err_str[50];
    	sprintf(err_str, "bad_addr_7 %d\n", addr);
        send_debug_message(err_str);
    }
}
//------------------------------------------------------------
void set_pixel_matrix_8(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_8)
    {
        matrix8[addr] = color;
    }
    else
    {
    	char err_str[50];
    	sprintf(err_str, "bad_addr_8 %d\n", addr);
        send_debug_message(err_str);
    }
}
//------------------------------------------------------------
bool set_left_pixel_front(unsigned int addr, RGB_t color)
{
    unsigned int addr1 = SIZE_MATRIX_7;
    unsigned int addr2 = addr1 + SIZE_MATRIX_1;
    unsigned int addr3 = addr2 + SIZE_MATRIX_6;

    if (addr < addr1)
    {
        set_pixel_matrix_7(addr, color);
        return true;
    }
    if (addr < addr2)
    {
        set_pixel_matrix_1(addr - addr1, color);
        return true;
    }
    if (addr < addr3)
    {
        set_pixel_matrix_6(addr - addr2, color);
        return true;
    }
    return false;
}
//------------------------------------------------------------
bool set_right_pixel_front(unsigned int addr, RGB_t color)
{
    unsigned int addr1 = SIZE_MATRIX_0;
    unsigned int addr2 = addr1 + SIZE_MATRIX_5;
    unsigned int addr3 = addr2 + SIZE_MATRIX_8;

    if (addr < addr1)
    {
        set_pixel_matrix_0(addr, color);
        return true;
    }
    if (addr < addr2)
    {
        set_pixel_matrix_5(addr - addr1, color);
        return true;
    }
    if (addr < addr3)
    {
        set_pixel_matrix_8(addr - addr2, color);
        return true;
    }
    return false;
}
//------------------------------------------------------------
bool set_left_pixel_back(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_2)
    {
        set_pixel_matrix_2(addr, color);
        return true;
    }
    if (addr < (SIZE_MATRIX_2 + (SIZE_MATRIX_3 / 2)))
    {
        set_pixel_matrix_3(SIZE_MATRIX_3 - (addr - SIZE_MATRIX_2) - 1, color);
        return true;
    }
    return false;
}
//------------------------------------------------------------
bool set_right_pixel_back(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_4)
    {
        //FIXME ошибка в распайке жилетки
        //set_pixel_matrix_4(SIZE_MATRIX_4 - addr - 1, color);

        set_pixel_matrix_4(addr, color);
        //---
        return true;
    }
    if (addr < (SIZE_MATRIX_4 + (SIZE_MATRIX_3 / 2)))
    {
        set_pixel_matrix_3(addr - SIZE_MATRIX_4, color);
        return true;
    }
    return false;
}
//------------------------------------------------------------
bool set_left_pixel_shoulders(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_6)
    {
        set_pixel_matrix_6(addr, color);
        return true;
    }
    return false;
}
//------------------------------------------------------------
bool set_right_pixel_shoulders(unsigned int addr, RGB_t color)
{
    if (addr < SIZE_MATRIX_8)
    {
        set_pixel_matrix_8(addr, color);
        return true;
    }
    return false;
}
//------------------------------------------------------------
void pause(int value)
{
	delay_ms(value);
}
//------------------------------------------------------------
