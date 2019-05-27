//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
#ifndef USART_H
#define USART_H
//--------------------------------------------------------------------------------
#define RX_BUF_SIZE 80

extern volatile char RX_FLAG_END_LINE;
extern volatile char RX_buffer[];
extern volatile unsigned char RX_buffer_len;
extern volatile char TX_buffer[];
extern volatile unsigned char TX_buffer_len;

void usart_init(void);

//--------------------------------------------------------------------------------
#endif
