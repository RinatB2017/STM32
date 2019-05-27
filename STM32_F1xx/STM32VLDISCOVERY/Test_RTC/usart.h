#include "stm32f10x_usart.h"
#include "stm32f10x_RCC.h"
#include "stm32f10x_GPIO.h"
#include <stdio.h>

void usart_init(void);
int sendchar(int ch);
int fputc(int ch, FILE *f);


unsigned char input_uasrt[7];
unsigned char atten;

void USART1_IRQHandler (void);
