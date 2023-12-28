#include "usart_console.h"
#include "usart.h"
#include "device.h"
uart_t uart3;
uint8_t tx_buffer[UART_TX_BUF];
ring_buffer_t tx_ring_buf = {.buffer = tx_buffer, .size = UART_TX_BUF, .flag_overflow = 0, .flag_error_pop = 0};

/* function to use printf() */
int _write(int file, char *ptr, int len)
{
  uint16_t ind = 0;
  while (ind<len)
  {
    ring_push(ptr[ind],&tx_ring_buf);
    ind++;
  }
  LL_USART_TransmitData8(USART3, ring_pop(&tx_ring_buf));
  return len;
}

void console_handler(void)
{
  if(uart_is_str_ready())
  {
    if(IS_CMD_RECIVED("wdt")) 
    {
      while(1)
      {
        ;
      }
    }else
    {
      if(console_cmd()==false)
      {
        printf("Unknowned cmd %s"NLINE"Cmd list:"NLINE"wdt"CONSOLE_HELP_CMD,(char*)&uart3.rx_buffer[0]);
      }
    }
    uart_rx_buff_reset();
  }
}

uint8_t is_cmd_recived(char* string, uint32_t size)
{
  uint8_t result = false;
  if(strncmp(string,(const char*)&uart3.rx_buffer[0], (size-1))==0)
  {
    result = true;
  }
  return result;
}
   

void uart_buff_put_char(uint8_t data)
{
  if(uart3.last_indx < UART_RX_BUF)
  {
    uart3.rx_buffer[uart3.last_indx++] = data;
    uart3.last_upd_ms = get_time_ms();
  }else{
    uart_rx_buff_reset();
  }
}

void uart_rx_buff_reset(void)
{
  uart3.last_indx = 0u;
}

uint8_t uart_is_str_ready(void)
{
  uint8_t result = false;
  if(time_elapsed_ms(uart3.last_upd_ms, 200) && uart3.last_indx!=0)
  {
    result = true;
  }
  return result;
}

int32_t get_int_arg(uint8_t* ptr)
{
  int32_t val = atoi((const char*)ptr);
  printf("Updated to = %ld"NLINE,val);
  return val;
}

float get_float_arg(uint8_t* ptr)
{
  float val = atof((const char*)ptr);
  printf("Updated to = %5.3f"NLINE,val);
  return val;
}
