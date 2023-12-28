
#ifndef USART_CONSOLE_H
#define USART_CONSOLE_H
#include "main.h"
#include "usart.h"
#include "ring_buffer.h"

#define TIME_BETWEEN_CMD_MS 200
#define IS_CMD_MATCH(string)\
    is_cmd_match(string, sizeof(string))

typedef struct{
  ring_buffer_t rx_fifo;
  ring_buffer_t tx_fifo;
  uint32_t rx_fifo_upd_ms;
}uart_t;

extern uart_t uart3;

void console_handler(void);
uint8_t uart_is_cmd_ready(void);
int32_t get_int_arg(uint8_t* ptr);
float get_float_arg(uint8_t* ptr);
uint8_t is_cmd_match(char* string, uint32_t size);
#endif /* USART_CONSOLE_H */
