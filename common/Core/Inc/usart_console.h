
#ifndef USART_CONSOLE_H
#define USART_CONSOLE_H
#include "main.h"
#include "usart.h"
#include "ring_buffer.h"

#define IS_CMD_RECIVED(string)\
    is_cmd_recived(string, sizeof(string))

typedef struct{
  uint8_t rx_buffer[UART_RX_BUF];
  uint8_t last_indx;
  uint32_t last_upd_ms;
}uart_t;

extern ring_buffer_t tx_ring_buf;
extern uart_t uart3;

void console_handler(void);
uint8_t is_cmd_recived(char* string, uint32_t size);
int32_t get_int_arg(uint8_t* ptr);
float get_float_arg(uint8_t* ptr);
void uart_buff_put_char(uint8_t data);
uint8_t uart_is_str_ready(void);
void uart_rx_buff_reset(void);
#endif /* USART_CONSOLE_H */
