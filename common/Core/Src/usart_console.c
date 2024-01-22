#include "usart_console.h"
#include "internal_flash.h"
#include "stm32f1xx_it.h"
#include "usart.h"
#include "device.h"
uint8_t tx_buf[UART_TX_BUF];
uint8_t rx_buf[UART_RX_BUF];
uart_t  uart3 = {.rx_fifo.buffer = rx_buf, .rx_fifo.size = UART_RX_BUF, .rx_fifo.flag_overflow = 0, .rx_fifo.flag_error_pop = 0,
                 .tx_fifo.buffer = tx_buf, .tx_fifo.size = UART_TX_BUF, .tx_fifo.flag_overflow = 0, .tx_fifo.flag_error_pop = 0,
                 .transfer_completed = true};

/* function to use printf() */
int _write(int file, char *ptr, int len)
{
  uint16_t ind = 0;
  while (ind<len)
  {
    ring_push(ptr[ind],&uart3.tx_fifo);
    ind++;
  }
  if(uart3.transfer_completed)
  {
    LL_USART_TransmitData8(USART3, ring_pop(&uart3.tx_fifo));
    uart3.transfer_completed = false;
  }
  return len;
}

void console_handler(void)
{
  if(uart_is_cmd_ready())
  {
    if(IS_CMD_MATCH("wdt")) 
    {
      while(1)
      {
        ;
      }
#ifdef CHECK_STACK_HEAP
    }else if(IS_CMD_MATCH("stack")){
      stack_heap_print_st();
#endif /* CHECK_STACK_HEAP */
    }else if(IS_CMD_MATCH("hwer")){
      if(*((uint8_t*)REBOOT_INFO_AREA) == 0xFF )
      {
        printf("No hw faults in memory"NLINE);
      }else if(*((uint8_t*)REBOOT_INFO_AREA) == STACK_OVERFLOW_ERROR)
      {
        printf("STACK OVERFLOW error stored in memory"NLINE);
      }else if(*((uint8_t*)REBOOT_INFO_AREA) == HEAP_OVERFLOW_ERROR)
      {
        printf("HEAP OVERFLOW error stored in memory"NLINE);

      }else{
        printf("UNKNOWN HW error stored in memory"NLINE);
      }

    }else if(IS_CMD_MATCH("clhwe")){
      internal_flash_unlock();
      internal_flash_Erase(REBOOT_INFO_AREA);
      internal_flash_lock();
      /* Next, we will have a reboot. The reason is WWDG */
    }else if(IS_CMD_MATCH("time?")){
      uint32_t s_raw = get_time_ms()/1000;
      uint32_t h,m,s; 
      h = s_raw/3600;
      m = (s_raw%3600)/60;
      s = ((s_raw%3600)%60);
      if(h<=9)printf("0");
      printf("%1ld:",h);
      if(m<=9)printf("0");
      printf("%1ld:",m);
      if(s<=9)printf("0");
      printf("%1ld"NLINE,s);
    }else if (IS_CMD_MATCH("fw_st?") || IS_CMD_MATCH("_fws")){
#ifdef BOOTLOADER
      printf("BL" NLINE);
#else
      printf("APP" NLINE);
#endif
    }else
    {
      if(console_cmd()==false)
      {
        printf("Unknowned cmd"NLINE
        "Cmd list:"
        NLINE"wdt"
#ifdef CHECK_STACK_HEAP
        NLINE"stack"
#endif /* CHECK_STACK_HEAP */
        NLINE"hwer"
        NLINE"clhwe"
        NLINE"time?"
        NLINE"fw_st?"
#if FW_NAME == BOOTLOADER
        NLINE"go_to_app"
#else
        NLINE"go_to_bl"
#endif        
        CONSOLE_HELP_CMD);
      }
    }
    ring_clear(&uart3.rx_fifo);
  }
}

uint8_t is_cmd_match(char* string, uint32_t size)
{
  uint8_t result = false;
  if(strncmp(string,(const char*)uart3.rx_fifo.buffer, (size-1))==0)
  {
    result = true;
  }
  return result;
}

uint8_t is_cmd_match_with_crc(char* string, uint32_t size)
{
  uint8_t result = false;
  
  if(hex_crc_check(uart3.rx_fifo.buffer, ring_get_count(&uart3.rx_fifo)-NLINE_SIZE))
  {
    result = is_cmd_match(string, size);
  }
  return result;
}

uint8_t uart_is_cmd_ready(void)
{
  uint8_t result = false;
  if(time_elapsed_ms(uart3.rx_fifo_upd_ms, TIME_BETWEEN_CMD_MS) && ring_get_count(&uart3.rx_fifo)!=0)
  {
    result = true;
  }
  return result;
}

int32_t get_int_arg(uint8_t* ptr, bool print_answ)
{
  int32_t val = atoi((const char*)ptr);
  if(print_answ){
    printf("%ld"NLINE,val);
  }
  return val;
}

float get_float_arg(uint8_t* ptr, bool print_answ)
{
  float val = atof((const char*)ptr);
  if(print_answ){
    printf("%5.3f"NLINE,val);
  }
  return val;
}

uint8_t hex_crc_check(uint8_t *ptr, uint32_t size)
{
    uint8_t sum = 0;
    for (int i = 0; i < size; i++)
    {
        sum += (uint8_t)ptr[i];
    }
    return (sum==0)?1:0;
}
