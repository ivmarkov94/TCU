
#ifndef USART_CONSOLE_H
#define USART_CONSOLE_H
#include "main.h"
#include "usart.h"
#include "ring_buffer.h"

#define  NLINE "\r\n"
#define  NLINE_SIZE 2
#define TIME_BETWEEN_CMD_MS 200
#define IS_CMD_MATCH(string)\
    is_cmd_match(string, sizeof(string))

#define IS_CMD_MATCH_WITH_CRC(string)\
    is_cmd_match_with_crc(string, sizeof(string))

typedef struct{
  ring_buffer_t rx_fifo;
  ring_buffer_t tx_fifo;
  uint32_t rx_fifo_upd_ms;
  uint8_t transfer_completed;
}uart_t;

extern uart_t uart3;

void console_handler(void);
uint8_t uart_is_cmd_ready(void);
int32_t get_int_arg(uint8_t* ptr, bool print_answ);
float get_float_arg(uint8_t* ptr, bool print_answ);
uint8_t is_cmd_match(char* string, uint32_t size);
uint8_t hex_crc_check(uint8_t *ptr, uint32_t size);
uint8_t is_cmd_match_with_crc(char* string, uint32_t size);
#endif /* USART_CONSOLE_H */
