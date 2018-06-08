#include "stm32f10x_usart.h"
#include "stm32f10x_RCC.h"
#include "stm32f10x_GPIO.h"
#include <stdio.h>

void usart_init(void);
void sendchar(int ch);
int fputc(int ch, FILE *f);

void usart_put_buf (unsigned char *buf, int buf_len);
void usart_put_str (unsigned char *buf);
